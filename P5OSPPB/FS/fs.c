typedef void* (*drv_func)(void*);
typedef void* (*drv_func2)(void*, void*);

typedef struct fsdriver {
	unsigned char type;
	drv_func dir_list;
	drv_func file_list;
	drv_func dir_del;
	drv_func dir_add;
	drv_func file_del;
	drv_func file_add;
	drv_func2 file_open;
	drv_func file_close;
	drv_func file_writeb;
	drv_func file_readb;
} fsdriver;

int fs_attach(unsigned char type, block_dev device, unsigned char* point);
int fs_detatch(block_dev device);
