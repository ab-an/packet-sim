#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <math.h>

#define N_PKTS 3
#define ID_SIZE 8 // bits
#define DATA_SIZE 8 // bits
#define N_BITS (N_PKTS * (ID_SIZE + DATA_SIZE))

struct pkt {
    uint8_t id[ID_SIZE];
    uint8_t data[DATA_SIZE];
};

float randn();
void printVec_u8(uint8_t *p, int N);
void printVec_float(float *p, int N);
void printPktBits(struct pkt *pktStore, int numPkts);

int main()
{
    // Initalize packet memory
    struct pkt pktStore[N_PKTS];

    // Initialize rng seed
    srand(time(NULL));

    // Generate a few packets
    for (int ii = 0; ii < N_PKTS; ii++)
    {
        // ID
        for (int jj = 0; jj < ID_SIZE; jj++)
        {
            pktStore[ii].id[jj] = (ii >> jj) & 1;
        }

        // Payload
        for(int jj = 0; jj < DATA_SIZE; jj++)
        {
            pktStore[ii].data[jj] = rand() % 2;
        }
    }

    // Print packet contents
    printPktBits(pktStore, 2);

    // Serialize packets to bitstream
    uint8_t bitstream[N_BITS];
    for (int ii = 0; ii < N_PKTS; ii++)
    {
        for (int jj = 0; jj < ID_SIZE; jj++)
        {
            bitstream[ii*(ID_SIZE+DATA_SIZE) + jj] = pktStore[ii].id[ID_SIZE - jj - 1];
        }
        for (int jj = 0; jj < DATA_SIZE; jj++)
        {
            bitstream[ii*(ID_SIZE+DATA_SIZE) + ID_SIZE + jj] = pktStore[ii].data[jj];
        }
    }

    // Print bitstream contents
    printf("Bitstream: ");
    printVec_u8(bitstream, N_BITS);
    printf("\n\n");

    // Map the bitstream to symbols
    float tx[N_BITS];
    for (int ii = 0; ii < N_BITS; ii++)
    {
        tx[ii] = bitstream[ii] ? 1.0 : -1.0; // BPSK
    }

    // Print symbols
    printf("Clean symbols: ");
    printVec_float(tx, N_BITS);

    // Let's add noise
    float rx[N_BITS];
    float noisePow = 0.04;
    float sigma = sqrt(noisePow);
    for(int ii = 0; ii < N_BITS; ii++)
    {
        rx[ii] = tx[ii] + sigma*randn();
    }

    // Print noisy symbols
    printf("\n\nNoisy symbols: ");
    printVec_float(rx, N_BITS);
    printf("\n\n");

    // Decode bits
    uint8_t decodes[N_BITS];
    for(int ii = 0; ii < N_BITS; ii++)
    {
        decodes[ii] = rx[ii] > 0 ? 1 : 0;
    }

    // Print decoded symbols
    printf("\n\nDecoded bits: ");
    printVec_u8(decodes, N_BITS);
    printf("\n");

    // Let's calculate BER
    int bitErrSum = 0;
    for(int ii = 0; ii < N_BITS; ii++)
    {
        bitErrSum += decodes[ii] != bitstream[ii];
    }
    float BER = (float)bitErrSum / N_BITS;

    printf("Bit errors: %d\n", bitErrSum);
    printf("\n\n Bit Error rate: %.3f\n", BER);

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

void printPktBits(struct pkt *pktStore, int numPkts)
{
    for (int ii=0; ii < numPkts; ii++)
    {
        printf("Packet %d, ID: \t", ii+1);
        for (int jj = 0; jj < ID_SIZE; jj++)
        {
            printf("%u ", pktStore[ii].id[ID_SIZE - jj - 1]);
        }
        printf("\n");
        printf("Packet %d, Data: ", ii+1);
        for (int jj = 0; jj < DATA_SIZE; jj++)
        {
            printf("%u ", pktStore[ii].data[jj]);
        }
        printf("\n\n");
    }
}

