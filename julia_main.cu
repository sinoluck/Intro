#include <stdio.h>
#include <math.h>

#include "bmp_class.h"

class Complex_double
{
public:
	double real;
	double img;
public:
	Complex_double()
	{
		real = 0;
		img = 0;
	}
	Complex_double(double a1,double a2)
	{
		real = a1;
		img = a2;
	}
	friend Complex_double operator*(const Complex_double&,const Complex_double&);
	friend Complex_double operator+(const Complex_double&,const Complex_double&);
	friend Complex_double operator-(const Complex_double&,const Complex_double&);

	double modsize()
	{
		return sqrt(real*real + img*img);
	}
};

Complex_double operator*(const Complex_double&a1,const Complex_double&a2)
{
	return Complex_double(a1.real*a2.real-a1.img*a2.img,a1.real*a2.img+a1.img+a2.real);
}
Complex_double operator+(const Complex_double&a1,const Complex_double&a2)
{
	return Complex_double(a1.real+a2.real,a1.img+a2.img);
}
Complex_double operator-(const Complex_double&a1,const Complex_double&a2)
{
	return Complex_double(a1.real-a2.real,a1.img-a2.img);
}
void print_usage()
{
	printf("usage:./julia width height real img file_name\n");
	exit(1);
}
__device__ int julia_map(int julia_n)
{

	int B,G,R;
	int AB = 0x1,BB = 0x12;
	int AG = 0x12,BG = 0x2;
	int AR = 0x4,BR = 0x20;

	B = AB*julia_n + BB;
	if(B > 0x0ff)B = 0x0ff;
	G = AG*julia_n + BG;
	if(G > 0x0ff)G = 0x0ff;
	R = AR*julia_n + BR;
	if(R > 0x0ff)R = 0x0ff;

	return B*0x010000+G*0x0100+R;
}

__global__ void julia_compute(int*n_tab,int img_width,int img_height,double c_real,double c_img)
{
	int tid = blockDim.x*blockIdx.x + threadIdx.x;
	//printf("tid:%d\n",tid);
	int x_id = tid%img_width;
	int y_id = tid/img_height;

	//compute

	//julia
	double zn_real = ((double)x_id/img_height-0.5)*2-0.74;
	double zn_img = ((double)y_id/img_width-0.5)*3;

	//mandelbrot
	/*
	c_real = (double)x_id/img_height-0.5;
	c_img = (double)y_id/img_width-0.5;
	double zn_real = 0;
	double zn_img = 0;
	*/
	if(tid < img_height*img_width)
	{
		int MAX_N = 2000;

		double MAX_MOD = 2.0;
		MAX_MOD = MAX_MOD*MAX_MOD;

		int n = 0; 
		for(n = 0;n < MAX_N;n++)
		{
			//Julia
			double tmp_real = zn_real*zn_real - zn_img*zn_img + c_real;
			double tmp_img = zn_img*zn_real*2 + c_img;
			if((tmp_real*tmp_real + tmp_img*tmp_img) > MAX_MOD)break;
			zn_real = tmp_real;
			zn_img = tmp_img;

		}
		n_tab[tid] = julia_map(n);
	}
}



void gpu_julia_compute(bmp_class &julia_ins,int img_width,int img_height,Complex_double c_const)
{
	//malloc cpu output
	int *cpu_output;
	cpu_output = (int*)malloc(sizeof(int)*img_width*img_height);
	if(cpu_output == NULL)
	{
		exit(-1);
	}
	//malloc gpu output
	int *gpu_output;
	cudaMalloc((void**)&gpu_output,sizeof(int)*img_width*img_height);

	//launch kernel
	int BLOCK_SIZE = 512;
	int block_num = (img_width*img_height+BLOCK_SIZE-1)/BLOCK_SIZE;
	julia_compute<<<block_num,BLOCK_SIZE>>>(gpu_output,img_width,img_height,c_const.real,c_const.img);

	//mov data back
	cudaMemcpy((void*)cpu_output,(const void*)gpu_output,sizeof(int)*img_width*img_height,cudaMemcpyDeviceToHost);

	//write to julia set
	int count = 0;
	for(int i = 0;i < img_width;i++)
	{
		for(int j = 0;j < img_height;j++)
		{

			//if(cpu_output[j*img_width+i] >= 2000)count++;
			int color_tmp = cpu_output[j*img_width+i];
			julia_ins.datatab[j][i] = color_tmp;
		}
	}
	printf("Overflow ratio:%.3f\n",(count+0.0)/img_width/img_height);
	cudaFree(gpu_output);

}

int main(int argc,char**argv)
{
	int img_width = 600;
	int img_height = 400;

	if(argc != 6)print_usage();
	img_width = atoi(argv[1]);
	img_height = atoi(argv[2]);
	double c_real = atof(argv[3]);
	double c_img = atof(argv[4]);

	Complex_double c_const(c_real, c_img);
	bmp_class julia_set;
	julia_set.resize(img_width,img_height);

	//double MAX = 200000.0;
	//double MAX_n = 2000;
	/*
	//cpu code
	for(int i = 0;i < img_height;i++)
	{
		for(int j = 0;j < img_width;j++)
		{
			double x_idx = (double)i/img_height;
			double y_idx = (double)j/img_width;
			Complex_double tmp(x_idx,y_idx);
			int n;
			for(n = 0;n < MAX_n;n++)
			{
				tmp = tmp*tmp-c_const;
				if(tmp.modsize() > MAX)break;	
			}

			printf("i:%3d,j:%3d,n:%4d\n",i,j,n);
			if(n <= MAX_n/3)julia_set.datatab[i][j] = 0x00ffff;
			else julia_set.datatab[i][j] = 0;
		}
	}
	*/

	//gpu version
	for(long i = 0;i < 20000000000;i++)
	{
		gpu_julia_compute(julia_set,img_width,img_height,c_const);
	}
	//julia_set.bmp_writedown(argv[5]);
	return 0;
}