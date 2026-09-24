/**
         shakeout2_util.c
**/

#include <string.h>

#include "ucvm_model_dtypes.h"
#include "shakeout2.h"
#include "um_netcdf.h"

#include "shakeout2_dataset.h"
#include "shakeout2_util.h"

/**** for shakeout2_dataset_t ****/
shakeout2_dataset_t *make_a_shakeout2_dataset(char *datadir, char *datafile, int tooBig, int useBinary) {
    char filepath[256];
    size_t nelems= 0;
    nc_type vtype;

    shakeout2_dataset_t *data=(shakeout2_dataset_t *)malloc(sizeof(shakeout2_dataset_t));

    sprintf(filepath, "%s/%s", datadir, datafile);
    if(shakeout2_ucvm_debug) fprintf(stderrfp," data file ..%s\n", filepath);

/* setup ncid */
    data->ncid=open_nc(filepath);
  
/* setup nx/ny/nz and void ptrs */
    data->longitudes=(float *) get_nc_float_buffer(data->ncid, "longitude", filepath, &vtype, &nelems, 1);
    data->nx=nelems;
    if(shakeout2_ucvm_debug_detail) {
        fprintf(stderrfp, "  Longitudes: %ld\n", nelems);
        for(int i=0;i<nelems; i++) {
            fprintf(stderrfp, "%d  %f\n", i, data->longitudes[i]);
       	}
    }

    data->latitudes=(float *) get_nc_float_buffer(data->ncid, "latitude", filepath, &vtype, &nelems, 1);
    data->ny=nelems;
    if(shakeout2_ucvm_debug_detail) {
        fprintf(stderrfp, "  Latitude: %ld\n", nelems);
        for(int i=0;i<nelems; i++) {
            fprintf(stderrfp, "%d  %f\n", i, data->latitudes[i]);
       	}
    }

    data->depths=(float *) get_nc_float_buffer(data->ncid, "depth", filepath, &vtype, &nelems, 1);
    data->nz=nelems;
    if(shakeout2_ucvm_debug_detail) {
        fprintf(stderrfp, "  Depths: %ld\n", nelems);
        for(int i=0;i<nelems; i++) {
            fprintf(stderrfp, "%d  %f\n", i, data->depths[i]);
       	}
    }

/* Get variable ID by name */
    data->vp_varid=get_nc_varid(data->ncid,"vp",filepath);
    data->vs_varid=get_nc_varid(data->ncid,"vs",filepath);
    data->rho_varid=get_nc_varid(data->ncid,"rho",filepath);

    data->layer_cache_cnt=0;
    data->col_cache_cnt=0;
    data->in_memory =0;

    if(!tooBig) {
/* load all vp/vs/rho data in memory */
        int total= data->nx * data->ny * data->nz;

	if(!useBinary) {
            if(shakeout2_ucvm_debug) { fprintf(stderrfp, " import USING netcdf data files\n"); }
            data->vp_buffer=get_nc_float_buffer(data->ncid, "vp", filepath, &vtype, &nelems, 3);
            data->vs_buffer=get_nc_float_buffer(data->ncid, "vs", filepath, &vtype, &nelems, 3);
            data->rho_buffer=get_nc_float_buffer(data->ncid, "rho", filepath, &vtype, &nelems, 3);
	    } else {
                if(shakeout2_ucvm_debug) { fprintf(stderrfp, " import USING binary data files\n"); }
                data->vp_buffer = get_binary_float_buffer(datadir, "vp.dat", total);
                data->vs_buffer = get_binary_float_buffer(datadir, "vs.dat", total);
                data->rho_buffer = get_binary_float_buffer(datadir, "rho.dat", total);
        }
	data->elems=total;
        data->in_memory=1;

        } else {
	  data->elems=0;
	  data->vp_buffer=NULL;
	  data->vs_buffer=NULL;
	  data->rho_buffer=NULL;
    }
    return data;
}



int free_shakeout2_dataset(shakeout2_dataset_t *data) {
    free(data->depths);
    free(data->latitudes);
    free(data->longitudes);
    if(data->vp_buffer != NULL) free(data->vp_buffer);
    if(data->vs_buffer != NULL) free(data->vs_buffer);
    if(data->rho_buffer != NULL) free(data->rho_buffer);
    close_nc(data->ncid);

    //free the caches
    for(int i=0; i< data->layer_cache_cnt; i++) {
      free_a_cache_layer(data->layer_cache[i]);
    }
    for(int i=0; i< data->col_cache_cnt; i++) {
      free_a_cache_col(data->col_cache[i]);
    }

    free(data);
    return SUCCESS;
}

/**** straight or trilinear/bilinear ****/
int _buffer_offset(shakeout2_dataset_t * dataset, int x_idx, int  y_idx, int z_idx) {
    int nx=dataset->nx;
    int ny=dataset->ny;
    int nz=dataset->nz;

    int offset= (z_idx)*(ny * nx)+(y_idx)*(nx)+x_idx;
    if(shakeout2_ucvm_debug) { fprintf(stderrfp,"\nTarget offset %d : idx lon/lat/dep = %d/%d/%d\n", offset,x_idx, y_idx, z_idx); }

    return offset;
}

int get_one_property(shakeout2_dataset_t *dataset, shakeout2_pt_info_t *pt, shakeout2_properties_t *data) {
    int offset= _buffer_offset(dataset, pt->lon_idx, pt->lat_idx, pt->dep_idx);

    data->vp=dataset->vp_buffer[offset];
    data->vs=dataset->vs_buffer[offset];
    data->rho=dataset->rho_buffer[offset];
    return offset;
}

float _interp_a_point(shakeout2_dataset_t *dataset, float *buffer, shakeout2_pt_info_t *pt) {
    int lon_idx=pt->lon_idx;
    int lat_idx=pt->lat_idx;
    int dep_idx=pt->dep_idx;
    float lon_percent=pt->lon_percent;
    float lat_percent=pt->lat_percent;
    float dep_percent=pt->dep_percent;

    if(pt->lon_idx < 0 || pt->lat_idx < 0 || pt->dep_idx < 0 ||
         pt->lon_idx +1 >= dataset->nx || pt->lat_idx +1 >= dataset->ny || pt->dep_idx+1 >= dataset->nz ) {
        // out of bound
        return -1;	
    }

    float val0= buffer[_buffer_offset(dataset,lon_idx,lat_idx,dep_idx)];      // x,    y, z
    float val1= buffer[_buffer_offset(dataset,lon_idx+1,lat_idx,dep_idx)];    // x+1,  y, z 
    float val2= buffer[_buffer_offset(dataset,lon_idx,lat_idx+1,dep_idx)];    // x,  y+1, z 
    float val3= buffer[_buffer_offset(dataset,lon_idx+1,lat_idx+1,dep_idx)];  // x+1,y+1, z
    float val4= buffer[_buffer_offset(dataset,lon_idx,lat_idx,dep_idx+1)];    // x,    y, z+1
    float val5= buffer[_buffer_offset(dataset,lon_idx+1,lat_idx,dep_idx+1)];  // x+1,  y, z+1
    float val6= buffer[_buffer_offset(dataset,lon_idx,lat_idx+1,dep_idx+1)];  // x,  y+1, z+1
    float val7= buffer[_buffer_offset(dataset,lon_idx+1,lat_idx+1,dep_idx+1)];// x+1,y+1, z+1
        
    float val00= val0 * (1-lon_percent) + val1 * lon_percent;    
    float val11= val4 * (1-lon_percent) + val5 * lon_percent;    
    float val22= val2 * (1-lon_percent) + val3 * lon_percent;    
    float val33= val6 * (1-lon_percent) + val7 * lon_percent;    

    float val000 = val00 * (1-lat_percent) + val22 * lat_percent;
    float val111 = val11 * (1-lat_percent) + val33 * lat_percent;

    float val0000 = val000 * (1-dep_percent) + val111 * dep_percent;
    return val0000;
}

void get_interp_property(shakeout2_dataset_t *dataset, shakeout2_pt_info_t *pt, shakeout2_properties_t *data) {

    if(shakeout2_ucvm_debug) { fprintf(stderrfp,"\nInterp PROCESSING for vp\n"); }
    data->vp = _interp_a_point(dataset, dataset->vp_buffer, pt);
    if(shakeout2_ucvm_debug) { fprintf(stderrfp,"\nInterp PROCESSING for vs\n"); }
    data->vs = _interp_a_point(dataset, dataset->vs_buffer, pt);
    if(shakeout2_ucvm_debug) { fprintf(stderrfp,"\nInterp PROCESSING for rho\n"); }
    data->rho = _interp_a_point(dataset, dataset->rho_buffer, pt);
    return;
}


/**** for shakeout2_cache_col_t ****/
shakeout2_cache_col_t *_add_a_cache_col(shakeout2_dataset_t *dataset, int target_lat_idx, int target_lon_idx) {
 
   int nz=dataset->nz;

   shakeout2_cache_col_t *col= (shakeout2_cache_col_t *) malloc(sizeof(shakeout2_cache_col_t));

   col->cache_col_lat_idx=target_lat_idx;
   col->cache_col_lon_idx=target_lon_idx;

   // alloc space first 
   col->col_vp_buffer=(float *) malloc(nz * sizeof(float));
   col->col_vs_buffer=(float *) malloc(nz * sizeof(float));
   col->col_rho_buffer=(float *) malloc(nz * sizeof(float));

   cache_depth_col_float(dataset->ncid, dataset->vp_varid,
                        nz, target_lat_idx, target_lon_idx, col->col_vp_buffer);
   cache_depth_col_float(dataset->ncid, dataset->vs_varid,
                        nz, target_lat_idx, target_lon_idx, col->col_vs_buffer);
   cache_depth_col_float(dataset->ncid, dataset->rho_varid,
                        nz, target_lat_idx, target_lon_idx, col->col_rho_buffer);
   return col;
}

shakeout2_cache_col_t *find_a_cache_col(shakeout2_dataset_t *dataset, int target_lat_idx, int target_lon_idx) {

   int cnt= dataset->col_cache_cnt; 
   shakeout2_cache_col_t *col;

   for(int i=0; i< cnt; i++) {
     col=dataset->col_cache[i];
     if((col->cache_col_lat_idx == target_lat_idx) &&
		      (col->cache_col_lon_idx == target_lon_idx) ) {
        // found it
        return col;
     }
   }
   // load it from the netcdf file
   col=_add_a_cache_col(dataset, target_lat_idx, target_lon_idx);
    
   // find a space to put in (in case it is full)
   if( cnt < SHAKEOUT2_CACHE_COL_MAX) {
       dataset->col_cache[cnt]=col;
       dataset->col_cache_cnt=cnt+1;
       } else {
// else has to free one out
         int use_idx=(cnt+1) % SHAKEOUT2_CACHE_COL_MAX;
         free_a_cache_col(dataset->col_cache[use_idx]);
         dataset->col_cache[use_idx]=col;

   }

   return col;     
}

void free_a_cache_col(shakeout2_cache_col_t *col) {

   // free buffer first 
   free(col->col_vp_buffer);
   free(col->col_vs_buffer);
   free(col->col_rho_buffer);

   free(col);
}

/**** for shakeout2_cache_layer_t ****/
shakeout2_cache_layer_t *_add_a_cache_layer(shakeout2_dataset_t *dataset, int target_dep_idx) {

    int nx=dataset->nx;
    int ny=dataset->ny;

    if(shakeout2_ucvm_debug) { fprintf(stderrfp, "  Loading a new layer: %d\n", target_dep_idx); }
    shakeout2_cache_layer_t *layer= (shakeout2_cache_layer_t *) malloc(sizeof(shakeout2_cache_layer_t));

    layer->cache_layer_dep_idx=target_dep_idx;

    layer->layer_vp_buffer = (float * )malloc((nx*ny) * sizeof(float));
    layer->layer_vs_buffer = (float * )malloc((nx*ny) * sizeof(float));
    layer->layer_rho_buffer = (float * )malloc((nx*ny) * sizeof(float));

    cache_latlon_layer_float(dataset->ncid, dataset->vp_varid,
                      target_dep_idx, ny, nx, layer->layer_vp_buffer);
    cache_latlon_layer_float(dataset->ncid, dataset->vs_varid,
                      target_dep_idx, ny, nx, layer->layer_vs_buffer);
    cache_latlon_layer_float(dataset->ncid, dataset->rho_varid,
                      target_dep_idx, ny, nx, layer->layer_rho_buffer);
    return layer;
}

shakeout2_cache_layer_t *find_a_cache_layer(shakeout2_dataset_t *dataset, int target_dep_idx) {

   int cnt= dataset->layer_cache_cnt;
   shakeout2_cache_layer_t *layer;

   for(int i=0; i< cnt; i++) {
     layer=dataset->layer_cache[i];
     if(layer->cache_layer_dep_idx == target_dep_idx) {
        // found it
        return layer;
     }
   }
   // load it from the netcdf file
   layer=_add_a_cache_layer(dataset, target_dep_idx);

   // find a space to put in (in case it is full)
   if( cnt < SHAKEOUT2_CACHE_LAYER_MAX) {
       dataset->layer_cache[cnt]=layer;
       dataset->layer_cache_cnt=cnt+1;
       } else {
// else has to free one out
         int use_idx=(cnt+1) % SHAKEOUT2_CACHE_LAYER_MAX;
         free_a_cache_layer(dataset->layer_cache[use_idx]);
         dataset->layer_cache[use_idx]=layer;
   }
   return layer;     
}

void free_a_cache_layer(shakeout2_cache_layer_t *layer) {

   free(layer->layer_vp_buffer);
   free(layer->layer_vs_buffer);
   free(layer->layer_rho_buffer);

   free(layer);
}
