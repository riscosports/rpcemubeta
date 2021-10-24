extern void adf_init(void);

extern void adf_load(int drive, const char *fn, int sectors, int size, int dblside, int dblstep, int density, int skew);

extern void adf_close(int drive);
extern void adf_seek(int drive, int track);
extern void adf_readsector(int drive, int sector, int track, int side, int density);
extern void adf_writesector(int drive, int sector, int track, int side, int density);
extern void adf_readaddress(int drive, int sector, int side, int density);
extern void adf_format(int drive, int sector, int side, int density);
extern void adf_stop(void);
extern void adf_poll(void);

#define disc_seek        adf_seek
#define disc_readsector  adf_readsector
#define disc_writesector adf_writesector
#define disc_readaddress adf_readaddress
#define disc_format      adf_format
#define disc_stop        adf_stop
#define disc_poll        adf_poll
