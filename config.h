/* After the reconfiguration, clean project. Otherwise the value could not renewed to the kernel code */
#define KERNEL_LEN 64 //Convolution kernel length
#define SEQUENCE_LEN 6400000 //Sequence length
#define SUB_SEQUENCE_LEN 64 //Length of partial sequence in input sequence 2

#define NUMs SEQUENCE_LEN/SUB_SEQUENCE_LEN
#define PARA 200 //The number of partial sequences in each reading

#define OUT_LEN NUMs * (KERNEL_LEN + SUB_SEQUENCE_LEN - 1) //the length of output sequence
#define M KERNEL_LEN //Convolution kernel length
#define N SUB_SEQUENCE_LEN //Length of partial sequence in input sequence 2

