/**
 * @file shakeout2_dataset.h
 *
**/

#ifndef SHAKEOUT2_DATASET_H
#define SHAKEOUT2_DATASET_H

// setup for future possible expansion
#define SHAKEOUT2_DATASET_MAX 1

#define SHAKEOUT2_CACHE_LAYER_MAX 20
#define SHAKEOUT2_CACHE_COL_MAX 10

typedef struct shakeout2_properties_t shakeout2_properties_t;

typedef struct shakeout2_cache_layer_t {
     int cache_layer_dep_idx;
     float *layer_vp_buffer; // nx * ny
     float *layer_vs_buffer;
     float *layer_rho_buffer;
} shakeout2_cache_layer_t;


typedef struct shakeout2_cache_col_t {
     int cache_col_lat_idx;
     int cache_col_lon_idx;
     float *col_vp_buffer; // nz
     float *col_vs_buffer;
     float *col_rho_buffer;
} shakeout2_cache_col_t;


/** The SHAKEOUT2 a dataset's working structure. */
typedef struct shakeout2_dataset_t {
	/** tracking netcdf id **/
        int ncid;

	/** Number of x(lon) points */
	int nx;
	/** Number of y(lat) points */
	int ny;
	/** Number of z(dep) points */
	int nz;

	/** list of longitudes **/
	float *longitudes;
	/** list of latitudes **/
	float *latitudes;
	/** list of depths **/
	float *depths;

	int vp_varid;
	int vs_varid;
	int rho_varid;

	int elems;
	float *vp_buffer;
	float *vs_buffer;
	float *rho_buffer; 

	// track how many cached layers
        int layer_cache_cnt;
        shakeout2_cache_layer_t *layer_cache[SHAKEOUT2_CACHE_LAYER_MAX];

/* a cache of previous column from cache_depth_col_float call */
	int col_cache_cnt;
        shakeout2_cache_col_t *col_cache[SHAKEOUT2_CACHE_COL_MAX];

/* flag to show if data i read in memory */
        int in_memory;

} shakeout2_dataset_t;

typedef struct shakeout2_pt_info_t {
	float lon;
	float lat;
	float dep;
        int lon_idx;
	int lat_idx;
	int dep_idx;
	float lon_percent;
	float lat_percent;
	float dep_percent;
} shakeout2_pt_info_t;

/* utilitie functions */
shakeout2_dataset_t *make_a_shakeout2_dataset(char *datadir, char *datafile, int tooBig, int useBinary);
int free_shakeout2_dataset(shakeout2_dataset_t *data);
shakeout2_cache_col_t *find_a_cache_col(shakeout2_dataset_t *dataset, int target_lat_idx, int target_lon_idx);
void free_a_cache_col(shakeout2_cache_col_t *col);
shakeout2_cache_layer_t *find_a_cache_layer(shakeout2_dataset_t *dataset, int target_dep_idx);
void free_a_cache_layer(shakeout2_cache_layer_t *layer);

int get_one_property(shakeout2_dataset_t *dataset, shakeout2_pt_info_t *pt, shakeout2_properties_t *data);
int get_one_shakeout21d_property(shakeout2_dataset_t *dataset, shakeout2_pt_info_t *pt, shakeout2_properties_t *data);
void get_interp_property(shakeout2_dataset_t *dataset, shakeout2_pt_info_t *pt, shakeout2_properties_t *data);


#endif

