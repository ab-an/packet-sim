#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>

#define N_PKTS 10000
#define ID_SIZE 8 // bits
#define DATA_SIZE 8 // bits
#define N_BITS (N_PKTS * (ID_SIZE + DATA_SIZE))
#define N_SWEEP 10
#define MIN_SNR -10
#define MAX_SNR 10

struct pkt {
    uint8_t id[ID_SIZE];
    uint8_t data[DATA_SIZE];
};

float randn();
void printVec_u8(uint8_t *p, int N);
void printVec_float(float *p, int N);
void printPktBits(struct pkt *pktMem, int numPkts);

int main()
{
    // Initialize rng seed
    srand(time(NULL));

    // Initialize SNR vector
    float SNR_dB[N_SWEEP];
    for(int ii = 0; ii < N_SWEEP; ii++)
    {
        SNR_dB[ii] = MIN_SNR + ii*(MAX_SNR - MIN_SNR)/(float)(N_SWEEP-1);
    }
    printf("SNR_dB: \n");
    printVec_float(SNR_dB, N_SWEEP);

    // Initialize BER metric
    float BER[N_SWEEP] = {0};

    // Sweep across SNR
    for(int ss = 0; ss < N_SWEEP; ss++)
    {
        // Initalize packet memory
        struct pkt pktMem[N_PKTS];

        // Generate packets
        for (int ii = 0; ii < N_PKTS; ii++)
        {
            // ID
            for (int jj = 0; jj < ID_SIZE; jj++)
            {
                pktMem[ii].id[jj] = (ii >> jj) & 1;
            }

            // Payload
            for(int jj = 0; jj < DATA_SIZE; jj++)
            {
                pktMem[ii].data[jj] = rand() % 2;
            }
        }

        // Serialize packets to bitstream
        uint8_t bitstream[N_BITS];
        for (int ii = 0; ii < N_PKTS; ii++)
        {
            for (int jj = 0; jj < ID_SIZE; jj++)
            {
                bitstream[ii*(ID_SIZE+DATA_SIZE) + jj] = pktMem[ii].id[ID_SIZE - jj - 1];
            }
            for (int jj = 0; jj < DATA_SIZE; jj++)
            {
                bitstream[ii*(ID_SIZE+DATA_SIZE) + ID_SIZE + jj] = pktMem[ii].data[jj];
            }
        }

        // Map the bitstream to symbols (TX)
        float tx[N_BITS];
        for (int ii = 0; ii < N_BITS; ii++)
        {
            tx[ii] = bitstream[ii] ? 1.0 : -1.0; // BPSK
        }

        // Compute and add noise (RX)
        float rx[N_BITS];
        float sigma = sqrt(1.0 / 2.0 / pow(10, SNR_dB[ss] / 10));
        for(int ii = 0; ii < N_BITS; ii++)
        {
            rx[ii] = tx[ii] + sigma*randn();
        }

        // Decode bits
        uint8_t decodes[N_BITS];
        for(int ii = 0; ii < N_BITS; ii++)
        {
            decodes[ii] = rx[ii] > 0 ? 1 : 0;
        }

        // Let's calculate BER
        int bitErrSum = 0;
        for(int ii = 0; ii < N_BITS; ii++)
        {
            bitErrSum += decodes[ii] != bitstream[ii];
        }
        float curr_BER = (float)bitErrSum / N_BITS;
        BER[ss] = curr_BER;

        printf("Bit errors: %d\n", bitErrSum);
        printf("\n\n Bit Error rate: %.3f\n", curr_BER);

    }

    FILE *fp = fopen("ber.csv", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }

    fprintf(fp, "SNR_dB,BER\n");
    for(int ii = 0; ii < N_SWEEP; ii++)
    {
        fprintf(fp, "%.2f,%e\n", SNR_dB[ii], BER[ii]);
    }
    fclose(fp);

    return 0;

}

float randn()
{
    float u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
    float u2 = (rand() + 1.0) / (RAND_MAX + 1.0);
    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

void printVec_u8(uint8_t *p, int N)
{
    for(int ii = 0; ii<N; ii++)
    {
        printf("%u ", *(p+ii));
    }
    printf("\n");
}

void printVec_float(float *p, int N)
{
    for(int ii = 0; ii<N; ii++)
    {
        printf("%.1f ", *(p+ii));
    }
    printf("\n");
}

void printPktBits(struct pkt *pktMem, int numPkts)
{
    for (int ii=0; ii < numPkts; ii++)
    {
        printf("Packet %d, ID: \t", ii+1);
        for (int jj = 0; jj < ID_SIZE; jj++)
        {
            printf("%u ", pktMem[ii].id[ID_SIZE - jj - 1]);
        }
        printf("\n");
        printf("Packet %d, Data: ", ii+1);
        for (int jj = 0; jj < DATA_SIZE; jj++)
        {
            printf("%u ", pktMem[ii].data[jj]);
        }
        printf("\n\n");
    }
}

