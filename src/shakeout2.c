/**
 * @file shakeout2.c
 * @brief Main file for shakeout2 model
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 */

#include <limits.h>
#include "ucvm_model_dtypes.h"
#include "shakeout2.h"
#include "shakeout2_dataset.h"
#include "shakeout2_util.h"
#include "um_netcdf.h"
#include "cJSON.h"

int shakeout2_ucvm_debug=0;
int shakeout2_ucvm_debug_detail=0;
FILE *stderrfp=NULL;

/** The config of the model */
char *shakeout2_config_string=NULL;
int shakeout2_config_sz=0;

// Constants
/** The version of the model. */
const char *shakeout2_version_string = "shakeout2";

// Variables
/** Set to 1 when the model is ready for query. */
int shakeout2_is_initialized = 0;

char shakeout2_data_directory[PATH_MAX];

/** Configuration parameters. */
shakeout2_configuration_t *shakeout2_configuration;
/** Holds pointers to the velocity model data OR indicates it can be read from file. */
shakeout2_model_t *shakeout2_velocity_model;

/**
 * Initializes the shakeout2 plugin model within the UCVM framework. In order to initialize
 * the model, we must provide the UCVM install path and optionally a place in memory
 * where the model already exists.
 *
 * @param dir The directory in which UCVM has been installed.
 * @param label A unique identifier for the velocity model.
 * @return Success or failure, if initialization was successful.
 */
int shakeout2_init(const char *dir, const char *label) {
    int tempVal = 0;
    char configbuf[512];
    double north_height_m = 0, east_width_m = 0, rotation_angle = 0;

    if(shakeout2_ucvm_debug) {
      stderrfp = fopen("shakeout2_debug.log", "w+");
      fprintf(stderrfp,"\n===== START shakeout2 ===== \n\n");
    }

    // Initialize variables.
    
    shakeout2_configuration = calloc(1, sizeof(shakeout2_configuration_t));
    shakeout2_config_string = calloc(SHAKEOUT2_CONFIG_MAX, sizeof(char));
    shakeout2_config_string[0]='\0';
    shakeout2_config_sz=0;

    shakeout2_velocity_model = calloc(1, sizeof(shakeout2_model_t));
    shakeout2_velocity_model_init(shakeout2_velocity_model);

    // Configuration file location.
    sprintf(configbuf, "%s/model/%s/data/config", dir, label);

    // Read the configuration file.
    if (shakeout2_read_configuration(configbuf, shakeout2_configuration) != UCVM_MODEL_CODE_SUCCESS) {
           // Try another, when is running in standalone mode..
       sprintf(configbuf, "%s/data/config", dir);
       if (shakeout2_read_configuration(configbuf, shakeout2_configuration) != UCVM_MODEL_CODE_SUCCESS) {
           shakeout2_print_error("No configuration file was found to read from.");
           return UCVM_MODEL_CODE_ERROR;
           } else {
           // Set up the data directory.
               snprintf(shakeout2_data_directory, sizeof(shakeout2_data_directory),  "%s/data/%s", dir, shakeout2_configuration->model_dir);
       }
       } else {
           // Set up the data directory.
           snprintf(shakeout2_data_directory, sizeof(shakeout2_data_directory),  "%s/model/%s/data/%s", dir, label, shakeout2_configuration->model_dir);
    }

    // Can we allocate the model, or parts of it, to memory. If so, we do.
    shakeout2_velocity_model->dataset_cnt=shakeout2_configuration->dataset_cnt;
    tempVal = shakeout2_read_model(shakeout2_configuration, shakeout2_velocity_model, shakeout2_data_directory);

    if (tempVal == SUCCESS) {
      if(shakeout2_ucvm_debug) {
        fprintf(stderrfp, "Setup model file\n");
      }
    } else if (tempVal == FAIL) {
        shakeout2_print_error("No model file was found to read from.");
        return FAIL;
    }

    // setup config_string 
    sprintf(shakeout2_config_string,"config = %s\n",configbuf);
    shakeout2_config_sz=1;

    // Let everyone know that we are initialized and ready for business.
    shakeout2_is_initialized = 1;

    return SUCCESS;
}

/**
 * Queries shakeout2 at the given points and returns the data that it finds.
 *
 * @param points The points at which the queries will be made.
 * @param data The data that will be returned (Vp, Vs, rho, Qs, and/or Qp).
 * @param numpoints The total number of points to query.
 * @return SUCCESS or FAIL.
 */
int shakeout2_query(shakeout2_point_t *points, shakeout2_properties_t *data, int numpoints) {

if(shakeout2_ucvm_debug){ fprintf(stderrfp,"\ncalling shakeout2_query with %d numpoints\n",numpoints); }

    int data_idx = 0;
    nc_type vtype=NC_FLOAT;

    /* iterate through the dataset to see where does the point fall into */
    /* for now assume there is only 1 dataset */
    shakeout2_dataset_t *dataset= shakeout2_velocity_model->datasets[data_idx];

    float *lon_list=dataset->longitudes;
    float *lat_list=dataset->latitudes;
    float *dep_list=dataset->depths;
    int nx=dataset->nx;
    int ny=dataset->ny;
    int nz=dataset->nz;

    // hold current working buffers
    float *tmp_vp_buffer=NULL;
    float *tmp_vs_buffer=NULL;
    float *tmp_rho_buffer=NULL;

    int first_lon_idx;
    int first_lat_idx;
    int first_dep_idx;

    int same_lon_idx=1;
    int same_lat_idx=1;
    int same_dep_idx=1;

    int lon_idx;
    int lat_idx;
    int dep_idx;

    int offset;

//  hold coord point's info
    shakeout2_pt_info_t *pt_info = (shakeout2_pt_info_t  *) malloc(numpoints * sizeof(shakeout2_pt_info_t));
    if (!pt_info) { fprintf(stderr, "pt_info: malloc failed\n");}

// iterate through all the points and compose the buffers for all index
    for(int i=0; i<numpoints; i++) {
        data[i].vp = -1;
        data[i].vs = -1;
        data[i].rho = -1;
        data[i].qp = -1;
        data[i].qs = -1;

        pt_info[i].lon=points[i].longitude;
        pt_info[i].lat=points[i].latitude;
        pt_info[i].dep=points[i].depth;

        pt_info[i].lon_idx=find_buffer_idx_clamped(lon_list,nx,pt_info[i].lon);
        pt_info[i].lat_idx=find_buffer_idx_clamped(lat_list,ny,pt_info[i].lat);
        pt_info[i].dep_idx=find_buffer_idx_clamped(dep_list,nz,pt_info[i].dep);

	/* check if out of range */
	if(pt_info[i].lon_idx < 0 || pt_info[i].lat_idx < 0 || pt_info[i].dep_idx < 0) {
          continue;
        }

        if(i==0) {
            first_dep_idx=pt_info[i].dep_idx;
            first_lon_idx=pt_info[i].lon_idx;
            first_lat_idx=pt_info[i].lat_idx;
        }

        if(pt_info[i].dep_idx != first_dep_idx) same_dep_idx=0;
        if(pt_info[i].lon_idx != first_lon_idx) same_lon_idx=0;
        if(pt_info[i].lat_idx != first_lat_idx) same_lat_idx=0;

	if(shakeout2_configuration->interpolation) { // fill cell percent
            pt_info[i].lon_percent=find_cell_percent(lon_list,pt_info[i].lon,pt_info[i].lon_idx);
            pt_info[i].lat_percent=find_cell_percent(lat_list,pt_info[i].lat,pt_info[i].lat_idx);
            pt_info[i].dep_percent=find_cell_percent(dep_list,pt_info[i].dep,pt_info[i].dep_idx);
        }
    }

// handle access 
// if not too_big, grab from in-memory buffer one at a time
// if too_big, then collect up all the index list and make just one call and
// retrieve and disperse the result back into data

    if(!shakeout2_configuration->too_big) { 

if(shakeout2_ucvm_debug){ fprintf(stderrfp,">> Using In-Memory access \n"); }

        // should be in the in-memory 
        for(int i=0; i<numpoints; i++) {
            if(!shakeout2_configuration->interpolation) { 
		// no interp
                get_one_property(dataset, &(pt_info[i]), &(data[i]));
                } else {
                    get_interp_property(dataset, &(pt_info[i]), &(data[i]));
            }
        }

    } else { 

//
// WARNING : no interpolation is done, for SHAKEOUT2, THIS SHOULD NOT BE USED 
// EXCEPT FOR TESTING
//
// It is too big, extract data from external data file 
// group netcdf access to 
//   column based(same lat idx an same lon idx),
//   or
//   layer based(same depth idx) 
//   or random method

// if same_lon_idx and same_lat_idx,
// it is depth profile
//    extract the whole column with dataset->nz for different set 
//    and then extract data from buffer one at a time using dep_idx 
        if(same_lon_idx && same_lat_idx ) {
          // grab a col from cache
            shakeout2_cache_col_t *col=find_a_cache_col(dataset, first_lat_idx, first_lon_idx); 
if(shakeout2_ucvm_debug){ fprintf(stderrfp,">> Using External data/Col cache - col idx=%d\n", dataset->col_cache_cnt); }
          
            tmp_vp_buffer=col->col_vp_buffer;
            tmp_vs_buffer=col->col_vs_buffer;
            tmp_rho_buffer=col->col_rho_buffer;

            for(int i=0; i<numpoints; i++) { 
                offset=pt_info[i].dep_idx;
                data[i].vp = tmp_vp_buffer[offset];
                data[i].vs = tmp_vs_buffer[offset];
                data[i].rho =tmp_rho_buffer[offset];
            }

// if just same_dep_idx, it is a horizontal slice,
// extract the whole layer using dataset->nx and dataset->ny
// and then extract data from buffer using lon_idx, and lat_idx
        } else if (same_dep_idx) { 
          // grab a layer from cache or try to load it from external data file
            shakeout2_cache_layer_t *layer=find_a_cache_layer(dataset, first_dep_idx);

if(shakeout2_ucvm_debug){ fprintf(stderrfp,">> Using External data/Layer cache - current count=%d\n", dataset->layer_cache_cnt); }
            tmp_vp_buffer=layer->layer_vp_buffer;
            tmp_vs_buffer=layer->layer_vs_buffer;
            tmp_rho_buffer=layer->layer_rho_buffer;

            for(int i=0; i<numpoints; i++) { 
                lat_idx=pt_info[i].lat_idx;
                lon_idx=pt_info[i].lon_idx;
                offset= (lat_idx * nx) + lon_idx;

                data[i].vp = tmp_vp_buffer[offset];
                data[i].vs = tmp_vs_buffer[offset];
                data[i].rho =tmp_rho_buffer[offset];
            }
        } else {  
// a very special case, it is random target ie. a vertical slice 
// handle it as random and so just default to per location access
if(shakeout2_ucvm_debug){ fprintf(stderrfp,">> Using External data/Random call \n"); }
            for(int i=0; i<numpoints; i++) { 
                lon_idx=pt_info[i].lon_idx;
                lat_idx=pt_info[i].lat_idx;
                dep_idx=pt_info[i].dep_idx;
                data[i].vp=get_nc_vara_float(dataset->ncid, dataset->vp_varid, dep_idx, lat_idx, lon_idx);
                data[i].vs=get_nc_vara_float(dataset->ncid, dataset->vs_varid, dep_idx, lat_idx, lon_idx);
                data[i].rho=get_nc_vara_float(dataset->ncid, dataset->rho_varid, dep_idx, lat_idx, lon_idx);
            }
         }
    }

    return SUCCESS;
}

/**
 */
void shakeout2_setdebug() {
   shakeout2_ucvm_debug=1;
}               

/**
 * Called when the model is being discarded. Free all variables.
 *
 * @return SUCCESS
 */
int shakeout2_finalize() {

    shakeout2_is_initialized = 0;

    if (shakeout2_configuration) {
        shakeout2_configuration_finalize(shakeout2_configuration);
    }
    if (shakeout2_velocity_model) {
        shakeout2_velocity_model_finalize(shakeout2_velocity_model);
    }

    if(shakeout2_ucvm_debug) {
     fprintf(stderrfp,"::DONE::\n"); 
     fclose(stderrfp);
    }

    return SUCCESS;
}

/**
 * Returns the version information.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int shakeout2_version(char *ver, int len)
{
  int verlen;
  verlen = strlen(shakeout2_version_string);
  if (verlen > len - 1) {
    verlen = len - 1;
  }
  memset(ver, 0, len);
  strncpy(ver, shakeout2_version_string, verlen);
  return 0;
}

/**
 * Returns the model config information.
 *
 * @param key Config key string to return.
 * @param sz number of config terms.
 * @return Zero
 */
int shakeout2_config(char **config, int *sz)
{
  int len=strlen(shakeout2_config_string);
  if(len > 0) {
    *config=shakeout2_config_string;
    *sz=shakeout2_config_sz;
    return SUCCESS;
  }
  return FAIL;
}


/**
 * Reads the shakeout2_configuration file describing the various properties of SHAKEOUT2 and populates
 * the shakeout2_configuration struct. This assumes shakeout2_configuration has been "calloc'ed" and validates
 * that each value is not zero at the end.
 *
 * @param file The shakeout2_configuration file location on disk to read.
 * @param config The shakeout2_configuration struct to which the data should be written.
 * @return Number of dataset expected in the model
 */
int shakeout2_read_configuration(char *file, shakeout2_configuration_t *config) {
    FILE *fp = fopen(file, "r");
    char key[40];
    char value[1000];
    char line_holder[2000];
    config->dataset_cnt=0;

    // If our file pointer is null, an error has occurred. Return fail.
    if (fp == NULL) { return UCVM_MODEL_CODE_ERROR; }

    // Read the lines in the shakeout2_configuration file.
    while (fgets(line_holder, sizeof(line_holder), fp) != NULL) {

        if (line_holder[0] != '#' && line_holder[0] != ' ' && line_holder[0] != '\n') {
            _splitline(line_holder, key, value);
            if(shakeout2_ucvm_debug){ fprintf(stderrfp, "a config line: %s\n", line_holder); }

            // Which variable are we editing?
            if (strcmp(key, "utm_zone") == 0) config->utm_zone = atoi(value);
            if (strcmp(key, "model_dir") == 0) snprintf(config->model_dir, sizeof(config->model_dir), "%s", value);
            if (strcmp(key, "interpolation") == 0) { 
                config->interpolation=0;
                if (strcmp(value,"on") == 0) { config->interpolation=1;
if(shakeout2_ucvm_debug){ fprintf(stderrfp, "enabled Interpolation\n"); }
                }
            }
            if (strcmp(key, "binary_data") == 0) { 
                config->use_binary=0;
                if (strcmp(value,"on") == 0) { config->use_binary=1;
if(shakeout2_ucvm_debug){ fprintf(stderrfp, "enabled Binary data\n"); }
                }
            }
            if (strcmp(key, "too_big") == 0) { 
                config->too_big=0;
                if (strcmp(value,"on") == 0) { config->too_big=1;
if(shakeout2_ucvm_debug){ fprintf(stderrfp, "enabled tooBig -- use external data access\n"); }
                }
            }
         /* for each dataset, allocate a model dataset's block and fill in */ 
            if (strcmp(key, "data_file") == 0) { 
                if( config->dataset_cnt < SHAKEOUT2_DATASET_MAX) {
                  int rc=_setup_a_dataset(config,value);
                  if(rc == 1 ) { config->dataset_cnt++; }
                } else { shakeout2_print_error("Exceeded dataset maximum limit."); }
	    } 
        }
    }

    // Have we set up all shakeout2_configuration parameters?
    if (config->utm_zone == 0 || config->model_dir[0] == '\0' || config->dataset_cnt == 0 ) {
        shakeout2_print_error("One shakeout2_configuration parameter not specified. Please check your shakeout2_configuration file.");
        return UCVM_MODEL_CODE_ERROR;
    }

    fclose(fp);
    return UCVM_MODEL_CODE_SUCCESS;
}

/**
 * Extract shakeout2 netcdf dataset specific info
 * and fill in the model info, one dataset at a time
 * and allocate required memory space
 *
 * @param 
 */
int _setup_a_dataset(shakeout2_configuration_t *config, char *blobstr) {
    /* parse the blob and grab the meta data */
    cJSON *confjson=cJSON_Parse(blobstr);
    int idx=config->dataset_cnt;

    /* grab the related netcdf file and extract dataset info */
    if(confjson == NULL) {
        const char *eptr = cJSON_GetErrorPtr();
        if(eptr != NULL) {
            if(shakeout2_ucvm_debug){ fprintf(stderrfp, "Configuration dataset setup error: (%s)\n", eptr); }
            return FAIL;
        }
    }

    cJSON *label = cJSON_GetObjectItemCaseSensitive(confjson, "LABEL");
    if(cJSON_IsString(label)){
        config->dataset_labels[idx]=strdup(label->valuestring);
    }
    cJSON *file = cJSON_GetObjectItemCaseSensitive(confjson, "FILE");
    if(cJSON_IsString(file)){
        config->dataset_files[idx]=strdup(file->valuestring);
    }
    return 1;
}

void _trimLast(char *str, char m) {
  int i;
  i = strlen(str);
  while (str[i-1] == m) {
    str[i-1] = '\0';
    i = i - 1;
  }
  return;
}

void _splitline(char* lptr, char key[], char value[]) {

  char *kptr, *vptr;

  for(int i=0; i<strlen(key); i++) { key[i]='\0'; }

  _trimLast(lptr,'\n');
  vptr = strchr(lptr, '=');
  int pos=vptr - lptr;

// skip space in key token from the back
  while ( lptr[pos-1]  == ' ') {
    pos--;
  }

  strncpy(key,lptr, pos);
  key[pos] = '\0';

  vptr++;
  while( vptr[0] == ' ' ) {
    vptr++;
  }
  strcpy(value,vptr);
  _trimLast(value,' ');
}


/*
 * setup the dataset's content 
 * and fill in the model info, one dataset at a time
 * and allocate required memory space
 *
 * */
int shakeout2_read_model(shakeout2_configuration_t *config, shakeout2_model_t *model, char *datadir) {

    int max_idx=model->dataset_cnt; // how many datasets are there
    for(int i=0; i<max_idx;i++) { 
        shakeout2_dataset_t *data=make_a_shakeout2_dataset(datadir, config->dataset_files[i], config->too_big, config->use_binary); 
        model->datasets[i]=data;
    }
    return SUCCESS;
}

/**
 * Called to clear out the allocated memory 
 *
 * @param 
 */
int shakeout2_configuration_finalize(shakeout2_configuration_t *config) {
    int max_idx=config->dataset_cnt; // how many datasets are there
    for(int i=0; i<max_idx;i++) { 
        free(config->dataset_labels[i]);  
        free(config->dataset_files[i]);  
    }
    free(config);
    return SUCCESS;
}

/**
 * Called to clear out the allocated memory for model datasets 
 *
 * @param 
 */

int shakeout2_velocity_model_finalize(shakeout2_model_t *model) {

    int max_idx=model->dataset_cnt; // how many datasets are there
    for(int i=0; i<max_idx; i++) {
        shakeout2_dataset_t *data= model->datasets[i];
        if(data != NULL) {
          free_shakeout2_dataset(data);
        }
    }
    return SUCCESS;
}
int shakeout2_velocity_model_init(shakeout2_model_t *model) {
    model->dataset_cnt=0;
    for(int i=0; i<SHAKEOUT2_DATASET_MAX; i++) {
        model->datasets[i]=NULL;
    }
    return SUCCESS;
}


/**
 * Prints the error string provided.
 *
 * @param err The error string to print out to stderr.
 */
void shakeout2_print_error(char *err) {
    fprintf(stderr, "An error has occurred while executing shakeout2. The error was: \n\n%s\n",err);
    fprintf(stderr, "\nPlease contact software@scec.org and describe both the error and a bit\n");
    fprintf(stderr, "about the computer you are running shakeout2 on (Linux, Mac, etc.).\n");
}

/*
 * Check if the data is too big to be loaded internally (exceed maximum
 * allowable by a INT variable)
 *
 */
static int data_too_big(shakeout2_dataset_t *dataset) {
    long max_size= (long) (dataset->nx) * dataset->ny * dataset->nz;
    long delta= max_size - INT_MAX;

    if( delta > 0) {
        return 1;
        } else {
            return 0;
        }
}

// The following functions are for dynamic library mode. If we are compiling
// a static library, these functions must be disabled to avoid conflicts.
#ifdef DYNAMIC_LIBRARY

/**
 * Init function loaded and called by the UCVM library. Calls shakeout2_init.
 *
 * @param dir The directory in which UCVM is installed.
 * @return Success or failure.
 */
int model_init(const char *dir, const char *label) {
    return shakeout2_init(dir, label);
}

/**
 * Query function loaded and called by the UCVM library. Calls shakeout2_query.
 *
 * @param points The basic_point_t array containing the points.
 * @param data The basic_properties_t array containing the material properties returned.
 * @param numpoints The number of points in the array.
 * @return Success or fail.
 */
int model_query(shakeout2_point_t *points, shakeout2_properties_t *data, int numpoints) {
    return shakeout2_query(points, data, numpoints);
}

/**
 * Finalize function loaded and called by the UCVM library. Calls shakeout2_finalize.
 *
 * @return Success
 */
int model_finalize() {
    return shakeout2_finalize();
}

/**
 * Version function loaded and called by the UCVM library. Calls shakeout2_version.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int model_version(char *ver, int len) {
    return shakeout2_version(ver, len);
}

/**
 * Version function loaded and called by the UCVM library. Calls shakeout2_config.
 *
 * @param config Config string to return.
 * @param sz number of config terms
 * @return Zero
 */
int model_config(char **config, int *sz) {
    return shakeout2_config(config, sz);
}


int (*get_model_init())(const char *, const char *) {
        return &shakeout2_init;
}
int (*get_model_query())(shakeout2_point_t *, shakeout2_properties_t *, int) {
         return &shakeout2_query;
}
int (*get_model_finalize())() {
         return &shakeout2_finalize;
}
int (*get_model_version())(char *, int) {
         return &shakeout2_version;
}
int (*get_model_config())(char **, int*) {
    return &shakeout2_config;
}



#endif
