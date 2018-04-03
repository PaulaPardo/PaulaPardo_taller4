#include <iostream>
#include <complex> 
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

using namespace std;

#define pi 3.141592

int numF(FILE *in);
void guardar(int nfil, FILE *in, float *t, float *x);
float lagrange(float *t,float tp,int j,int n);
float Pollagrange(float *t,float *x,float tp,int n);
float* darvalorT(float* T,float *t,int n);
float* darvalorX(float* X,float* T,float *t,float *x,int N);
float* darvalorF(float* F,float* T,int N);
float* fourierR(float* fft, float* F,float *x,int N);
float* fourierI(float* fft, float* F,float *x,int N);



int main(int argc,char *argv[])
{
        char *nombre;
        nombre = argv[1];
        FILE *in = fopen(nombre, "r");
	int nf = numF(in);
        int nf2 = nf/2;
        float L;
	in = fopen(nombre, "r");
	float *t = (float*)malloc((nf2)*sizeof(float));
	float *x = (float*)malloc((nf2)*sizeof(float));
	guardar(nf,in,t,x);
        float* tn;
        tn = darvalorT(tn,t,nf2);
        float* xn;
        xn = darvalorX(xn,tn,t,x,nf2);
        float* f;
        f = darvalorF(f,tn,nf2);
        float* FftR;
        FftR = fourierR(FftR,f,xn,nf2);
        float* FftI;
        FftI = fourierI(FftI,f,xn,nf2);
        FILE *out = fopen("transformada.txt", "w+");
        
        
        for (int i=0;i<nf2;i++)
        {
         fprintf(out,"%f %f %f \n",tn[i],xn[i],f[i]);
        }


        fclose(out);

}

int numF(FILE *in)
{ 
	float nu;
	int num=0;
	
	while(!feof(in))
	{
		fscanf(in, "%f \n", &nu);
		num++;
	}
	fclose(in);

	return num;
}


void guardar(int nfil, FILE *in, float *t, float *x)
{
	int n_t;
	int n_x;
	for(int i=0; i < nfil; i++)
	{	
		if((i%2)==0)
        	{
			fscanf(in, "%f \n", &t[n_t]);
        		n_t++;
        	}
                                        
        	else
        	{
        		fscanf(in, "%f \n", &x[n_x]);
			n_x++;
		}
	
	}
	fclose(in);
}

float lagrange(float *t,float tp,int j,int n)
{
    float l_j=1.0; 
    
    for (int i=0;i<n;i++)
    {  
        if(i != j)
        {
            l_j = l_j*((float)(tp - t[i])/(float)(t[j]-t[i]));
        }
    }
    return l_j;
}

float Pollagrange(float *t,float *x,float tp,int n)
{
    float L=0;

    for(int i=0; i<n; i++)
    {
        L += (x[i]*lagrange(t,tp,i,n));
    }
    return L;
}

float* darvalorT(float* T,float *t,int N)
{
    T = (float*) malloc(N*sizeof(float));
    for(int i=0;i<N;i++)
    {
        T[i] = t[0] + i*(t[N-1]-t[0])/((float)N-1.0);
    }
    return T;
}

float* darvalorX(float* X,float* T,float *t,float *x,int N)
{
    X = (float*) malloc(N*sizeof(float));
    for(int i=0;i<N;i++)
    {
        X[i] = Pollagrange(t,x,T[i],N);
    }
    return X;
}

float* darvalorF(float* F,float* T,int N)
{
    F = (float*) malloc(N*sizeof(float));
    for(int i=0;i<N;i++)
    {
        F[i] = i;
    }
    return F;
}

float* fourierR(float* fft, float* F,float *x,int N)
{
    fft = (float*) malloc(N*sizeof(float));
    for (int k = 0; k < N; k++)
    {  
        double suma = 0;
        for (int l = 0; l < N; l++) 
        {  
            double angle = 2 * pi * l * k / N;
            suma +=  x[l] * cos(angle);
        }
        fft[k] = suma;
    }
    return fft;
}
    


float* fourierI(float* fft, float* F,float *x,int N)
{
    fft = (float*) malloc(N*sizeof(float));
    for (int k = 0; k < N; k++)
    {  
        double suma2 = 0;
        for (int l = 0; l < N; l++) 
        {  
            double angle = 2 * pi * l * k / N;
            suma2 +=  -x[l] * sin(angle);
        }
        fft[k] = suma2;
    }
    return fft;
}



