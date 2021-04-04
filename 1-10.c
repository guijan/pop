#if 0
#define FTZMETER	0.3048
#define	METERZFT	3.28084
#define	MIZFT		5280.0
#define	MIZKM		1.609344
#define	SQMIZSQKM	2.589988
#endif

/* How would I write this? Exactly as it is. enum doesn't support floats.
 * Perhaps a series of const declarations would be better */
const float FTZMETER	= 0.3048;
const float METERZFT	= 3.28084;
const float MIZFT	= 5280.0;
const float MIZKM	= 1.609344;
const float SQMIZSQKM	= 2.589988;
