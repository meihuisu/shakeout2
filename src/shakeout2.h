/**
 * @file shakeout2.h
 * @brief Main header file for SHAKEOUT2 library.
 * @version 1.0
 *
 * Delivers the SHAKEOUT2 model 
 *
 */
#ifndef SHAKEOUT2_H
#define SHAKEOUT2_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "shakeout2_dataset.h"

/** Defines a return value of success */
#define SUCCESS 0
/** Defines a return value of failure */
#define FAIL 1

/* config string */
#define SHAKEOUT2_CONFIG_MAX 1000
#define SHAKEOUT2_DATASET_MAX 1

extern int shakeout2_ucvm_debug;
extern int shakeout2_ucvm_debug_detail;
extern FILE *stderrfp;

// Structures
/** Defines a point (latitude, longitude, and depth) in WGS84 format */
typedef struct shakeout2_point_t {
	/** Longitude member of the point */
	double longitude;
	/** Latitude member of the point */
	double latitude;
	/** Depth member of the point */
	double depth;
} shakeout2_point_t;

/** Defines the material properties this model will retrieve. */
typedef struct shakeout2_properties_t {
	/** P-wave velocity in meters per second */
	double vp;
	/** S-wave velocity in meters per second */
	double vs;
	/** Density in g/m^3 */
	double rho;
	/** Qp */
	double qp;
	/** Qs */
	double qs;
} shakeout2_properties_t;

/**
Dimensions: 3
  dim[0] name=depth len=84
  dim[1] name=latitude len=381
  dim[2] name=longitude len=471
Total elements: 15073884
**/

/** The SHAKEOUT2 configuration structure. */
typedef struct shakeout2_configuration_t {
	/** The zone of UTM projection */
	int utm_zone;
	/** The model directory */
	char model_dir[128];

	/** interpolation on or off (1 or 0) */
	int interpolation;
	/** use_binary on or off (1 or 0) */
	int use_binary;
	/** too_big on or off (1 or 0) */
	int too_big;

        /* how many datasets are in the model */
        int dataset_cnt;
        char *dataset_files[SHAKEOUT2_DATASET_MAX];  //strdup
	char *dataset_labels[SHAKEOUT2_DATASET_MAX]; // strdup
						  
} shakeout2_configuration_t;

typedef struct shakeout2_model_t {
        int dataset_cnt;
        shakeout2_dataset_t *datasets[SHAKEOUT2_DATASET_MAX];
} shakeout2_model_t;


// Constants
/** The version of the model. */
extern const char *shakeout2_version_string;

// Variables
/** Set to 1 when the model is ready for query. */
extern int shakeout2_is_initialized;

/** Configuration parameters. */
extern shakeout2_configuration_t *shakeout2_configuration;

/** Holds pointers to the velocity model data OR indicates it can be read from file. */
extern shakeout2_model_t *shakeout2_velocity_model;

// UCVM API Required Functions

#ifdef DYNAMIC_LIBRARY

/** Initializes the model */
int model_init(const char *dir, const char *label);
/** Cleans up the model (frees memory, etc.) */
int model_finalize();
/** Returns version information */
int model_version(char *ver, int len);
/** Queries the model */
int model_query(shakeout2_point_t *points, shakeout2_properties_t *data, int numpts);

int (*get_model_init())(const char *, const char *);
int (*get_model_query())(shakeout2_point_t *, shakeout2_properties_t *, int);
int (*get_model_finalize())();
int (*get_model_version())(char *, int);

#endif

// SHAKEOUT2 Related Functions

/** Initializes the model */
int shakeout2_init(const char *dir, const char *label);
/** Cleans up the model (frees memory, etc.) */
int shakeout2_finalize();
/** Returns version information */
int shakeout2_version(char *ver, int len);
/** Queries the model */
int shakeout2_query(shakeout2_point_t *points, shakeout2_properties_t *data, int numpts);

// Non-UCVM Helper Functions
//
/** Reads the configuration file and helper functions. */
int shakeout2_read_configuration(char *file, shakeout2_configuration_t *config);
int shakeout2_configuration_finalize(shakeout2_configuration_t *config);

/** Prints out the error string. */
void shakeout2_print_error(char *err);
/** Retrieves the value at a specified grid point in the model. */
void shakeout2_read_properties(int x, int y, int z, shakeout2_properties_t *data);
/** Attempts to malloc the model size in memory and read it in. */
int shakeout2_read_model(shakeout2_configuration_t *config, shakeout2_model_t *model, char* dir);
/** toggle debug flag **/
void shakeout2_setdebug();

/** helper function for velocity_model **/
int shakeout2_velocity_model_init(shakeout2_model_t *model);
int shakeout2_velocity_model_finalize(shakeout2_model_t *model);

/** parse JSON metadata blob per dataset **/
int _setup_a_dataset(shakeout2_configuration_t *conf, char *blobstr);

void _trimLast(char *str, char m);
void _splitline(char* lptr, char key[], char value[]);

#endif
