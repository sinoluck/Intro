
#include <stdio.h>
#include <stdlib.h>

#ifndef _BMP_CLASS_H_
#define _BMP_CLASS_H_



typedef struct BMPHEAD
{
	char headflag[2];
	unsigned int filesize;
	unsigned char res[4];
	unsigned int table_size;
}BMPHEAD_t;

typedef struct BMPDESBLOCK
{
	unsigned int blocksize;
	unsigned int width;
	unsigned int height;
	unsigned short int plane;
	unsigned short int digit;
	unsigned int ziptype;
	unsigned int datasize;
	unsigned int horpixel;
	unsigned int verpixel;
	unsigned int colornum;
}BMPDESBLOCK_t;

typedef struct colortab
{
	unsigned char B;
	unsigned char G;
	unsigned char R;
	unsigned char alpha;
}colortab_t;

typedef struct BMPCOLORTAB
{
	colortab_t *colortab_array;
}BMPCOLORTAB_t;

class bmp_class
{
public:
	BMPHEAD_t bmphead_ins;
	BMPDESBLOCK_t bmpdesblock_ins;
	BMPCOLORTAB_t bmpcolortab_ins;
	int **datatab;

public:
	bmp_class()
	{
		bmphead_ins.headflag[0] = 'B';
		bmphead_ins.headflag[1] = 'M';
		bmphead_ins.table_size = 54;

		bmpdesblock_ins.blocksize = 0x28;
		bmpdesblock_ins.width = 0;
		bmpdesblock_ins.height = 0;
		bmpdesblock_ins.plane = 1;
		bmpdesblock_ins.digit = 24;
		bmpdesblock_ins.ziptype = 0;
		bmpdesblock_ins.datasize = 0;
		bmpdesblock_ins.horpixel = 0;
		bmpdesblock_ins.verpixel = 1;
		bmpdesblock_ins.colornum = 0;
		bmpcolortab_ins.colortab_array = NULL;
		datatab = NULL;
	}
	bmp_class(const char *file_name)
	{
		FILE*file_ptr = NULL;
		//int flag = -1;
		while(1)
		{
			file_ptr = fopen(file_name,"r");
			if(file_ptr == NULL)
			{
				printf("%s file open failed\n",file_name);
				break;
			}

			fread((void*)bmphead_ins.headflag,sizeof(char),2,file_ptr);
			if((bmphead_ins.headflag[0] == 'B') && (bmphead_ins.headflag[1] == 'M'))
			{
				;
			}
			else break;

			fread((void*)(&bmphead_ins.filesize),sizeof(unsigned int),1,file_ptr);
			fread((void*)bmphead_ins.res,sizeof(unsigned char),4,file_ptr);
			fread((void*)(&bmphead_ins.table_size),sizeof(unsigned int),1,file_ptr);

			//printf("read file head succeeded\n");
			//read block
			fread((void*)&bmpdesblock_ins.blocksize,4,1,file_ptr);
			fread((void*)&bmpdesblock_ins.width,4,1,file_ptr);
			fread((void*)&bmpdesblock_ins.height,4,1,file_ptr);
			fread((void*)&(bmpdesblock_ins.plane),2,1,file_ptr);
			fread((void*)&bmpdesblock_ins.digit,2,1,file_ptr);
			fread((void*)&bmpdesblock_ins.ziptype,4,1,file_ptr);
			fread((void*)&bmpdesblock_ins.datasize,4,1,file_ptr);
			fread((void*)&bmpdesblock_ins.horpixel,4,1,file_ptr);
			fread((void*)&bmpdesblock_ins.verpixel,4,1,file_ptr);
			fread((void*)&bmpdesblock_ins.colornum,4,1,file_ptr);

			//printf("read block succeeded\n");
			//read color tab
			int color_tab_size;
			switch(bmpdesblock_ins.digit)
			{
				case 1:color_tab_size = 2;break;
				case 4:color_tab_size = 16;break;
				case 8:color_tab_size = 256;break;
				case 24:color_tab_size = 0;break;
				default:
				{
					printf("error\n");
				};break;
			}
			while(1)
			{
				if(color_tab_size == 0)
				{
					bmpcolortab_ins.colortab_array = NULL;
					break;
				}
				bmpcolortab_ins.colortab_array = (colortab_t*)malloc(sizeof(colortab)*color_tab_size);
				if(bmpcolortab_ins.colortab_array == NULL)
				{
					printf("out of memory\n");
					break;
				}
				int i = 0;
				for(i = 0;i < color_tab_size;i++)
				{
					fread((void*)&(bmpcolortab_ins.colortab_array[i].B),1,1,file_ptr);
					fread((void*)&(bmpcolortab_ins.colortab_array[i].G),1,1,file_ptr);
					fread((void*)&(bmpcolortab_ins.colortab_array[i].R),1,1,file_ptr);
					fread((void*)&(bmpcolortab_ins.colortab_array[i].alpha),1,1,file_ptr);
				}
			
			}			
			//printf("read color tab succeeded\n");
			//repos file pointer
			fseek(file_ptr,bmphead_ins.table_size,SEEK_SET);
			//read data
			datatab = NULL;
			datatab = (int**)malloc(sizeof(int*)*bmpdesblock_ins.height);
			if(datatab == NULL)printf("out of memory");
			int i = 0;
			for(i = 0;i < bmpdesblock_ins.height;i++)
			{
				datatab[i] = NULL;
				datatab[i] = (int*)malloc(sizeof(int)*bmpdesblock_ins.width);
				if(datatab[i] == NULL)
				{
					printf("out of memory\n");
				}
			}

			switch(bmpdesblock_ins.digit)
			{
				case 1:
				{
					int offset = 0;
					int tmp_pixel[8];
					int digit_per_line =(((bmpdesblock_ins.width+7)/8+3)/4)*4*8;
					for(offset = 0;;offset+=8)
					{
						unsigned char char_tmp;
						int ret_tmp = fread((void*)(&char_tmp),1,1,file_ptr);
						if(ret_tmp!=1)
						{
							break;
						}
						for(int i = 0;i < 8;i++)
						{
							tmp_pixel[i] = (((int)char_tmp)>>(7-i))&0x1;
							int tmp_id = offset+i;

							int idx = tmp_id%digit_per_line;
							int idy = tmp_id/digit_per_line;
							if(idx >= bmpdesblock_ins.width)continue;
							else
							{
								datatab[idy][idx] = tmp_pixel[i];
							}
						}
					}
				};break;
				case 4:
				{
					int offset = 0;
					int tmp_pixel[2];
					int digit_per_line = (((bmpdesblock_ins.width+1)/2+3)/4)*4*2;
					for(offset = 0;;offset +=8)
					{
						unsigned char char_tmp;
						int ret_tmp = fread((void*)&char_tmp,1,1,file_ptr);
						if(ret_tmp != 1)
						{
							break;
						}
						for(int i = 0;i < 2;i++)
						{
							tmp_pixel[i] = (((int)char_tmp)>>(4*(1-i)))&0x0f;
							int tmp_id = offset + i;
							int idx = tmp_id%digit_per_line;
							int idy = tmp_id/digit_per_line;
							if(idx >= bmpdesblock_ins.width)continue;
							else
							{
								datatab[idy][idx] = tmp_pixel[i];
							}
						}
					}
				};break;
				case 8:
				{
					int offset = 0;
					int tmp_pixel;
					int digit_per_line = ((bmpdesblock_ins.width+3)/4)*4;
					for(offset = 0;;offset++)
					{
						int ret_tmp = fread((void*)&tmp_pixel,1,1,file_ptr);
						if(ret_tmp != 1)
						{
							break;
						}
						int idx = offset%digit_per_line;
						int idy = offset/digit_per_line;
						if(idx >= bmpdesblock_ins.width)continue;
						else datatab[idy][idx]=tmp_pixel;
					}

				};break;
				case 24:
				{

					int offset = 0;
					unsigned char tmp_pixel[3];
					long int tmp_a = ftell(file_ptr);

					int digit_per_line = bmpdesblock_ins.width;
					int aligned_line_size = ((digit_per_line*3+3)/4)*4;
					for(offset = 0;;)
					{
						unsigned char tmp_read;
						if(offset%(aligned_line_size) >= (digit_per_line*3))
						{
							int ret_tmp = fread((void*)&tmp_read,1,1,file_ptr);
							if(ret_tmp == 0)break;
							offset ++ ;
							continue;
						}
						else
						{
							int ret_tmp = fread((void*)tmp_pixel,1,3,file_ptr);
							if(ret_tmp != 3) break;
							int tmp_idx = (offset%aligned_line_size)/3;
							int tmp_idy = offset/aligned_line_size;

							int tmp_data = 0;
							tmp_data += ((int)tmp_pixel[0])<<16;
							tmp_data += ((int)tmp_pixel[1])<<8;
							tmp_data += tmp_pixel[2];
							if((tmp_idx < digit_per_line) && (tmp_idy < bmpdesblock_ins.height))datatab[tmp_idy][tmp_idx] = tmp_data;
							offset += 3;
						}
					}


				};break;
				default:printf("error\n");
				;break;
			}
		break;
		}
		fclose(file_ptr);
		//printf("cs\n");
	}
	int bmp_writedown(const char*file_name)
	{
		FILE*file_ptr = fopen(file_name,"w");
		if(file_ptr == NULL)
		{
			printf("file open failed\n");
			return -1;
		}
		fwrite("BM",sizeof(char),2,file_ptr);

		//compute file size
		unsigned int file_size = 14+40;
		int tmp_c_tab_size = 0;
		switch(bmpdesblock_ins.digit)
		{	
			case 1:tmp_c_tab_size = 8;break;
			case 4:tmp_c_tab_size = 64;break;
			case 8:tmp_c_tab_size = 1024;break;
			case 24:tmp_c_tab_size = 0;break;
			default://printf("digit?\n");
			;break;
		}
		file_size += tmp_c_tab_size;
		//int tmp_datasize = 0;
		int tmp_width = bmpdesblock_ins.width;
		int tmp_height = bmpdesblock_ins.height;
		int size_per_line = 0;
		switch(bmpdesblock_ins.digit)
		{
			case 1:
			{
				size_per_line = ((((tmp_width+7)/8)+3)/4)*4;
			};break;
			case 4:
			{
				size_per_line = (((tmp_width+1)/2+3)/4)*4;
			};break;
			case 8:
			{
				size_per_line = ((tmp_width+3)/4)*4;
			};break;
			case 24:
			{
				size_per_line = ((tmp_width*3+3)/4)*4;
			};break;
			default:
			{
				size_per_line = 4;
			};break;
		}
		file_size += size_per_line*tmp_height;
		bmpdesblock_ins.datasize = size_per_line*tmp_height;
		fwrite((void*)&file_size,sizeof(unsigned int),1,file_ptr);
		fwrite("\0\0\0\0",sizeof(char),4,file_ptr);
		bmphead_ins.table_size = 54+tmp_c_tab_size;
		fwrite((void*)&bmphead_ins.table_size,sizeof(unsigned int),1,file_ptr);

		//write des block
		fwrite((void*)&bmpdesblock_ins.blocksize,sizeof(unsigned int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.width,sizeof(unsigned int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.height,sizeof(unsigned int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.plane,sizeof(unsigned short int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.digit,sizeof(unsigned short int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.ziptype,sizeof(unsigned int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.datasize,sizeof(unsigned int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.horpixel,sizeof(unsigned int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.verpixel,sizeof(unsigned int),1,file_ptr);
		fwrite((void*)&bmpdesblock_ins.colornum,sizeof(unsigned int),1,file_ptr);

		//write color tab
		int color_tab_size;
		switch(bmpdesblock_ins.digit)
		{
			case 1:color_tab_size = 2;break;
			case 4:color_tab_size = 16;break;
			case 8:color_tab_size = 256;break;
			case 24:color_tab_size = 0;break;
			default:
			{
				printf("error\n");
			};break;
		}
		for(int i = 0;i < color_tab_size;i++)
		{
			fwrite((void*)&bmpcolortab_ins.colortab_array[i].B,sizeof(unsigned char),1,file_ptr);
			fwrite((void*)&bmpcolortab_ins.colortab_array[i].G,sizeof(unsigned char),1,file_ptr);
			fwrite((void*)&bmpcolortab_ins.colortab_array[i].R,sizeof(unsigned char),1,file_ptr);
			fwrite((void*)&bmpcolortab_ins.colortab_array[i].alpha,sizeof(unsigned char),1,file_ptr);
		}

		//write data 
		//write alignment
		long cur_offset = ftell(file_ptr);
		for(;cur_offset < bmphead_ins.table_size;cur_offset++)
		{
			fwrite("\0",sizeof(char),1,file_ptr);
		}

		int x_max = bmpdesblock_ins.width;
		int y_max = bmpdesblock_ins.height;
		for(int y_ite = 0;y_ite < y_max;y_ite++)
		{
			switch(bmpdesblock_ins.digit)
			{
				case 1:
				{
					int tmp_pixel[8];
					for(int i = 0;i < 8;i++)tmp_pixel[i] = 0;
					for(int ite1 = 0;ite1 < bmpdesblock_ins.width;ite1+=8)
					{
						for(int ite2 = 0;ite2 < 8;ite2++)
						{
							int x_idx = ite1+ite2;
							if(x_idx > bmpdesblock_ins.width)break;
							tmp_pixel[ite2] = datatab[y_ite][x_idx]&0x1;
						}
						unsigned char data_tmp = 0;
						for(int ite2 = 0;ite2 < 8;ite2++)
						{
							data_tmp += (unsigned char)(tmp_pixel[ite2]<<(7-ite2));
						}
						fwrite((void*)&data_tmp,sizeof(unsigned char),1,file_ptr);
					}
					int ext_byte,align_byte_num;
					ext_byte = ((bmpdesblock_ins.width+7)/8);
					align_byte_num = ((ext_byte+3)/4)*4 - ext_byte;
					for(int i = 0;i < align_byte_num;i++)
					{
						fwrite("\0",sizeof(char),1,file_ptr);
					}
				}break;
				case 4:
				{
					int tmp_pixel[2];
					for(int i = 0;i < 2;i++)tmp_pixel[i] = 0;
					for(int ite1 = 0;ite1 < bmpdesblock_ins.width;ite1 += 2)
					{
						for(int ite2 = 0;ite2 < 2;ite2++)
						{
							int x_idx = ite1 + ite2;
							if(x_idx > bmpdesblock_ins.width)break;
							tmp_pixel[ite2] = datatab[y_ite][x_idx]&0x0f;
						}
						unsigned char data_tmp = 0;
						for(int ite2 = 0;ite2 < 2;ite2 ++)
						{
							data_tmp += (unsigned char)(tmp_pixel[ite2]<<((1-ite2)*4));
						}
						fwrite((void*)&data_tmp,sizeof(unsigned char),1,file_ptr);
					}
					int ext_byte,align_byte_num;
					ext_byte= ((bmpdesblock_ins.width+1)/2);
					align_byte_num = ((ext_byte+3)/4)*4-ext_byte;
					for(int i = 0;i < align_byte_num;i++)
					{
						fwrite("\0",sizeof(char),1,file_ptr);
					}
				}break;
				case 8:
				{
					for(int x_ite = 0;x_ite < x_max;x_ite++)
					{
						unsigned char data_tmp = (unsigned char)(datatab[y_ite][x_ite]&0x0ff);
						fwrite((void*)&data_tmp,sizeof(char),1,file_ptr);
					}
					int align_byte_num;
					align_byte_num = ((x_max+3)/4)*4-x_max;
					for(int i = 0;i < align_byte_num;i++)
					{
						fwrite("\0",sizeof(char),1,file_ptr);
					}

				}break;
				case 24:
				{
					for(int x_ite = 0;x_ite < x_max;x_ite++)
					{
						unsigned char data_tmp[3];
						data_tmp[0] = (unsigned char)((datatab[y_ite][x_ite]&0x00ff0000)>>16);
						data_tmp[1] = (unsigned char)((datatab[y_ite][x_ite]&0x0000ff00)>>8);
						data_tmp[2] = (unsigned char)(datatab[y_ite][x_ite]&0x000000ff);
						fwrite((void*)data_tmp,sizeof(char),3,file_ptr);
					}
					int align_byte_num;
					align_byte_num = ((x_max*3+3)/4)*4-x_max*3;
					for(int i = 0;i < align_byte_num;i++)
					{
						fwrite("\0",sizeof(char),1,file_ptr);
					}
					//long tmp = ftell(file_ptr);
					//printf("%ld\n",tmp);
				}break;
				default:
				{
					printf("failed\n");
					//return -1;
				}break;
			}
		}
		fclose(file_ptr);
		return 0;
	}
	~bmp_class()
	{
		if(bmpcolortab_ins.colortab_array!= NULL)
		free(bmpcolortab_ins.colortab_array);
		int y_max = bmpdesblock_ins.height;
		for(int i = 0;i < y_max;i++)free(datatab[i]);
		free(datatab);
	}

	int resize(int width,int height)
	{
		//free
		if(bmpcolortab_ins.colortab_array!= NULL)
		free(bmpcolortab_ins.colortab_array);
		int y_max = bmpdesblock_ins.height;
		for(int i = 0;i < y_max;i++)free(datatab[i]);
		free(datatab);
		//malloc
		bmpdesblock_ins.width = width;
		bmpdesblock_ins.height = height;
		datatab = (int**)malloc(sizeof(int*)*height);
		if(datatab == NULL)
		{
			return -1;
		}
		for(int i = 0;i < height;i++)
		{
			datatab[i] = (int*)malloc(sizeof(int)*width);
			if(datatab[i] == NULL)
			{
				return -1;
			}
		}
		return 0;
	}
	void print_data()
	{
		int x_max = bmpdesblock_ins.width;
		int y_max = bmpdesblock_ins.height;
		
		for(int j = 0;j < y_max;j++)
		{
			for(int i = 0;i < x_max;i++)
			{
				printf("0x%x,",datatab[j][i]);
			}
			printf("\n");
		} 
	}

};

#endif