julia:julia_main.cu bmp_class.h
	nvcc -o julia julia_main.cu -arch=compute_35