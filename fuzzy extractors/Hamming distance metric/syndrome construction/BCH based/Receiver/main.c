#include <stdio.h>
#include <stdint.h>

// #define m 8      //size of finite field = 2^m
#define n 255       //2^m-1
#define length 200  //codeword length
// #define k 168       //data length
#define t 4      	//error tolerance (maximum number of bit errors)
//The max tolerable mismatch rate = t/length

//GF(2^8) tables
int16_t alpha_to[n+1] = {1, 2, 4, 8, 16, 32, 64, 128, 113, 226, 181, 27, 54, 108, 216, 193, 243, 151, 95, 190, 13, 26, 52, 104, 208, 209, 211, 215, 223, 207, 239, 175, 47, 94, 188, 9, 18, 36, 72, 144, 81, 162, 53, 106, 212, 217, 195, 247, 159, 79, 158, 77, 154, 69, 138, 101, 202, 229, 187, 7, 14, 28, 56, 112, 224, 177, 19, 38, 76, 152, 65, 130, 117, 234, 165, 59, 118, 236, 169, 35, 70, 140, 105, 210, 213, 219, 199, 255, 143, 111, 222, 205, 235, 167, 63, 126, 252, 137, 99, 198, 253, 139, 103, 206, 237, 171, 39, 78, 156, 73, 146, 85, 170, 37, 74, 148, 89, 178, 21, 42, 84, 168, 33, 66, 132, 121, 242, 149, 91, 182, 29, 58, 116, 232, 161, 51, 102, 204, 233, 163, 55, 110, 220, 201, 227, 183, 31, 62, 124, 248, 129, 115, 230, 189, 11, 22, 44, 88, 176, 17, 34, 68, 136, 97, 194, 245, 155, 71, 142, 109, 218, 197, 251, 135, 127, 254, 141, 107, 214, 221, 203, 231, 191, 15, 30, 60, 120, 240, 145, 83, 166, 61, 122, 244, 153, 67, 134, 125, 250, 133, 123, 246, 157, 75, 150, 93, 186, 5, 10, 20, 40, 80, 160, 49, 98, 196, 249, 131, 119, 238, 173, 43, 86, 172, 41, 82, 164, 57, 114, 228, 185, 3, 6, 12, 24, 48, 96, 192, 241, 147, 87, 174, 45, 90, 180, 25, 50, 100, 200, 225, 179, 23, 46, 92, 184, 0};
int16_t index_of[n+1] = {-1, 0, 1, 231, 2, 207, 232, 59, 3, 35, 208, 154, 233, 20, 60, 183, 4, 159, 36, 66, 209, 118, 155, 251, 234, 245, 21, 11, 61, 130, 184, 146, 5, 122, 160, 79, 37, 113, 67, 106, 210, 224, 119, 221, 156, 242, 252, 32, 235, 213, 246, 135, 22, 42, 12, 140, 62, 227, 131, 75, 185, 191, 147, 94, 6, 70, 123, 195, 161, 53, 80, 167, 38, 109, 114, 203, 68, 51, 107, 49, 211, 40, 225, 189, 120, 111, 222, 240, 157, 116, 243, 128, 253, 205, 33, 18, 236, 163, 214, 98, 247, 55, 136, 102, 23, 82, 43, 177, 13, 169, 141, 89, 63, 8, 228, 151, 132, 72, 76, 218, 186, 125, 192, 200, 148, 197, 95, 174, 7, 150, 71, 217, 124, 199, 196, 173, 162, 97, 54, 101, 81, 176, 168, 88, 39, 188, 110, 239, 115, 127, 204, 17, 69, 194, 52, 166, 108, 202, 50, 48, 212, 134, 41, 139, 226, 74, 190, 93, 121, 78, 112, 105, 223, 220, 241, 31, 158, 65, 117, 250, 244, 10, 129, 145, 254, 230, 206, 58, 34, 153, 19, 182, 237, 15, 164, 46, 215, 171, 99, 86, 248, 143, 56, 180, 137, 91, 103, 29, 24, 25, 83, 26, 44, 84, 178, 27, 14, 45, 170, 85, 142, 179, 90, 28, 64, 249, 9, 144, 229, 57, 152, 181, 133, 138, 73, 92, 77, 104, 219, 30, 187, 238, 126, 16, 193, 165, 201, 47, 149, 216, 198, 172, 96, 100, 175, 87};

//Calculate the syndrome of BCH codeword
void getsyndrome(uint8_t codeword[], int16_t s[]){

	uint8_t i, j;

	for(i = 1; i <= 2*t; i++){		
		s[i] = 0;

		for(j = 0; j < length; j++){
			if (codeword[j] != 0){					
				s[i] ^= alpha_to[(i * j) % n];	
			}
		}
	}
}

//Correct the error using BCH decoding (Simon Rockliff's implementation of Berlekamp's algorithm)
void correcterr(uint8_t codeword[], int16_t s[]){

	uint16_t i, j, u, q;
    uint8_t count = 0;
    int16_t elp[2*t+2][t+1], du[2*t], l[2*t+1], u_l[2*t], reg[2*t];
    uint8_t loc[t];

	//Step 1: convert the syndrome into index form
	for (i = 1; i <= 2*t; i++) {		
		s[i] = index_of[s[i]];
	}

	//Step 2: calculate the error location polynomial
	du[0] = 0;			
	du[1] = s[1];		
	elp[0][0] = 0;		
	elp[1][0] = 1;		

	for(i = 1; i < 2*t; i++) {
		elp[0][i] = -1;	
		elp[1][i] = 0;	
	}

	l[0] = 0;
	l[1] = 0;
	u_l[0] = -1;
	u_l[1] = 0;
	
	u = 0;

	do {
		u++;
		if(du[u] == -1) {	
			l[u + 1] = l[u];
			for (i = 0; i <= l[u]; i++) {
				elp[u + 1][i] = elp[u][i];
				elp[u][i] = index_of[elp[u][i]];
			}
		}else
			
		{
			q = u - 1;
			while ((du[q] == -1) && (q > 0))
				q--;

			if(q > 0) {
			  	j = q;
			  	do{
			    	j--;
			    	if ((du[j] != -1) && (u_l[q] < u_l[j]))
			      		q = j;
			  	}while (j > 0);
			}

			if(l[u] > l[q] + u - q){
				l[u + 1] = l[u];
			}
			else{
				l[u + 1] = l[q] + u - q;
			}

			for(i = 0; i < 2*t; i++){
				elp[u + 1][i] = 0;		
			}
			for(i = 0; i <= l[q]; i++){
				if (elp[q][i] != -1){
					elp[u + 1][i + u - q] = alpha_to[(du[u] + n - du[q] + elp[q][i]) % n];
				}
			}
			for (i = 0; i <= l[u]; i++) {
				elp[u + 1][i] ^= elp[u][i];		
				elp[u][i] = index_of[elp[u][i]];
			}
		}
		u_l[u + 1] = u - l[u + 1];	

		if (u < 2*t) {	
			if (s[u + 1] != -1){
		    	du[u + 1] = alpha_to[s[u + 1]];
			}else{
				du[u + 1] = 0;
			}
		    for (i = 1; i <= l[u + 1]; i++){
		      	if ((s[u + 1 - i] != -1) && (elp[u + 1][i] != 0)){
		        	du[u + 1] ^= alpha_to[(s[u + 1 - i] + index_of[elp[u + 1][i]]) % n];
		      	}
		    }
		  	du[u + 1] = index_of[du[u + 1]];	
		}
	}while ((u < 2*t) && (l[u + 1] <= t));
	
	u++;

	if(l[u] <= t){
		for (i = 0; i <= l[u]; i++){
			elp[u][i] = index_of[elp[u][i]];	
		}

		//Step 3: Chien search - find roots of the error location polynomial
		for(i = 1; i <= l[u]; i++){
			reg[i] = elp[u][i];
		}
		count = 0;
		for(i = 1; i <= n; i++){
			q = 1;
			for(j = 1; j <= l[u]; j++){
				if (reg[j] != -1) {
					reg[j] = (reg[j] + j) % n;
					q ^= alpha_to[reg[j]];
				}
			}
			if(!q){	
				loc[count] = n - i;	
				count++;
			}
		}
		
		//Step 4: correct the malformed codeword
		if(count == l[u]){
			for (i = 0; i < l[u]; i++){
				codeword[loc[i]] ^= 1;	
			}
		}
	}
}


//RX of syndrome-based construction of fuzzy extractor
int main(){

	uint8_t i, errtag = 0;

	//Extracted PS bit string (uniformly distributed random). Assume it's composed of all 1s.
    uint8_t ps[length] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    //Received from TX
    int16_t securesketchTX[2*t+1] = {0, 249, 107, 200, 198, 205, 174, 18, 250};		//the received message contains the last 2*t elements only. The first element is 0 by default
    //Generated by RX
    int16_t securesketchRX[2*t+1];

    //add some mismatches to the PS bit string (for test only)
    ps[0] ^= 1;
    ps[50] ^= 1;
    ps[100] ^= 1;
    // ps[150] ^= 1;
    ps[199] ^= 1;

    getsyndrome(ps, securesketchRX);

    for(i = 1; i < 2*t+1; i++){
    	securesketchRX[i] ^= securesketchTX[i];
    	if(securesketchRX[i] != 0){
    		errtag = 1;
    	}
    }

    if(errtag){
    	//correct the errors
    	correcterr(ps, securesketchRX);
    }

    //Print the corrected PS measurements
    printf("The corrected PS string\n");
    for(i = 0; i < length; i++){
    	printf("%d, ", ps[i]);
    }
    printf("\n");

//////////////////////////////////////////////////////////////////

// //Strong extractor
}