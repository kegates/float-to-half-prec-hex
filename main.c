//parses data in output-data-txt folder to
//.MIF format
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef union{
  double f;
  long unsigned int i;
} RAW;

//converts double to half precision float
//assumes non-zero/undesired values
RAW dtof16(RAW val);

int main() {
  FILE *fp_txt;
  FILE *fp_mif;

  char fnm[60];
  char str[60];

  RAW hidden_w;
  RAW output_w;

  //Hidden Layer Weights
  for(int i = 1; i < 31; i++) {
      sprintf(fnm,"../output-data-txt/W784_%d", i);
      fp_txt = fopen(fnm,"r");
      if(fp_txt == NULL) {
        perror("Error opening txt file");
        return(-1);
      }
      sprintf(fnm,"../output-data-mif/W784_%d.mif", i);
      fp_mif = fopen(fnm, "w");
      if(fp_mif == NULL) {
        perror("Error opening mif file");
        return(-1);
      }

      fprintf(fp_mif,"WIDTH = 16;\n");
      fprintf(fp_mif,"DEPTH = 1024;\n\n");
      fprintf(fp_mif,"ADDRESS_RADIX = HEX;\n");
      fprintf(fp_mif,"DATA_RADIX = HEX;\n\n");
      fprintf(fp_mif, "CONTENT BEGIN\n");

      //actual filling of values
      for(int j = 0; j < 784; j++) {
        fgets(str,60,fp_txt);
        hidden_w.f = atof(str);
        fprintf(fp_mif,"%04X : %04X;\n",j,dtof16(hidden_w).i);
      }
      fprintf(fp_mif, "END;\n");

      fclose(fp_txt);
      fclose(fp_mif);
  }

  //Output Layer Weights
  for(int i = 1; i < 11; i++) {
      sprintf(fnm,"../output-data-txt/W30_%d", i);
      fp_txt = fopen(fnm,"r");
      if(fp_txt == NULL) {
        perror("Error opening txt file");
        return(-1);
      }
      sprintf(fnm,"../output-data-mif/W30_%d.mif", i);
      fp_mif = fopen(fnm, "w");
      if(fp_mif == NULL) {
        perror("Error opening mif file");
        return(-1);
      }

      fprintf(fp_mif,"WIDTH = 16;\n");
      fprintf(fp_mif,"DEPTH = 1024;\n\n");
      fprintf(fp_mif,"ADDRESS_RADIX = HEX;\n");
      fprintf(fp_mif,"DATA_RADIX = HEX;\n\n");
      fprintf(fp_mif, "CONTENT BEGIN\n");

      //actual filling of values
      for(int j = 0; j < 30; j++) {
        fgets(str,60,fp_txt);
        hidden_w.f = atof(str);
        fprintf(fp_mif,"%04X : %04X;\n",j,dtof16(hidden_w).i);
      }
      fprintf(fp_mif, "END;\n");

      fclose(fp_txt);
      fclose(fp_mif);
  }

  //biases in format to replace relevant section in dxp_dnn_digits1.v
    sprintf(fnm,"../output-data-txt/bias_hidden.txt");
    fp_txt = fopen(fnm,"r");
    if(fp_txt == NULL) {
      perror("Error opening txt file");
      return(-1);
    }
    sprintf(fnm,"../output-data-mif/biases-verilog.txt");
    fp_mif = fopen(fnm, "w");
    if(fp_mif == NULL) {
      perror("Error opening mif file");
      return(-1);
    }

    //actual filling of values
    for(int j = 1; j < 31; j++) {
      fgets(str,60,fp_txt);
      hidden_w.f = atof(str);
      fprintf(fp_mif,"\tassign hlb%d = 16'h%04x;\n",j,dtof16(hidden_w).i);
    }

    fclose(fp_txt);

    fprintf(fp_mif, "\n\tassign  hlsigout31 = 16'h0000; assign hlsigout32 = 16'h0000;\n\n");
    sprintf(fnm,"../output-data-txt/bias_out.txt");
    fp_txt = fopen(fnm,"r");
    if(fp_txt == NULL) {
      perror("Error opening txt file");
      return(-1);
    }
    //actual filling of values
    for(int j = 1; j < 11; j++) {
      fgets(str,60,fp_txt);
      hidden_w.f = atof(str);
      fprintf(fp_mif,"\tassign olb%d = 16'h%04x;\n",j,dtof16(hidden_w).i);
    }

    fclose(fp_txt);



    fclose(fp_mif);



}


RAW dtof16(RAW val) {
  RAW sign;
  RAW exp;
  RAW fract;
  RAW out;

  out.i = 0;

  sign.i = val.i >> 63;
  exp.i = (val.i >> 52) & 0x7ff;
  fract.i = (val.i << 12) >> 12;

  /*printf("In funct\n");
  printf("Original %x%x\n",val.i >> 32, val.i);
  printf("sign %x%x\n",sign.i >> 32, sign.i);
  printf("exp %x%x\n",exp.i >> 32, exp.i);
  printf("exp %u\n",exp.i);
  printf("fract %x%x\n",fract.i >> 32, fract.i);
*/
  //To convert, only take top 10 fraction bits,
  //sub back in bias to exp (1023) and add half-prec bias (15) then take
  // 5 LSbits
  //sign stays the same
  sign.i = sign.i;
  exp.i = exp.i - 1023 + 15;
  fract.i = fract.i >> 42;

/*
  printf("Post-Mod %x%x\n",val.i >> 32, val.i);
  printf("sign %x%x\n",sign.i >> 32, sign.i);
  printf("exp %x%x\n",exp.i >> 32, exp.i);
  printf("exp %u\n",exp.i);
  printf("fract %x%x\n",fract.i >> 32, fract.i);
*/
  out.i = out.i & 0b0;
  out.i |= fract.i << 15;
  out.i |= exp.i << 10;
  out.i |= fract.i;
  out.i &= 0xffff;

  //printf("Final %x\n", out.i);

  return out;
}
