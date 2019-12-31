/***
 * This code is due to: https://github.com/r2x0t by way of https://github.com/cnlohr/esptracker/issues/1.
 *
 * It's included here to have it in a state to kick around; it doesn't actually work quite right -- but it does work
 * and the reason it works is pretty interesting in and of itself. It turns out if you take a LFSR data stream and
 * encode it using differential manchester encoding, then decode that with a manchester decoder (instead of
 * manchester differential), it maps perfectly to a higher order LFSR. The higher order is necessary since you can
 * invert the modulated signal and manchester inverts; while diff manchester doesn't.
 *
 * It's possible this only holds for the 32 polynomials used in the lighthouse; but more likely this is a property
 * of all LFSR streams; while also saying something about the two types of encoding.
 *
 * I don't think this property is particularly useful; the lower order the polynomial the easier it is to work with
 * and the two decoders are of similar complexity.
 */

#include <stdio.h>
#include <stdint.h>

/*
_CHAN_INIT_Data_20180805155528.8.dat use polys 0xD3675,0x90C2D
_CHAN_AFTER_BUTTON_Data_20180805170303.8.dat uses 0xB85F3,0xC058B
*/

int main(int argc, char **argv, char **env)
{
 if (argc<3) {
  printf("[file] [ch]\n");
  return 0;
 }
 FILE *f = fopen(argv[1],"rb");
 int ch = 0;
 sscanf(argv[2],"%d",&ch);

 unsigned char buf[1024];
 int bitmask = 1<<ch;
 double fil[7] = {0};
 double old = 0;
 int dist = 0;

    int taps[] = {0xD3675,0x90C2D, // 8_seconds_8_bit_fixed_Data_20180805134454.8.dat _CHAN_INIT_Data_20180805155528.8.dat
               0xB85F3,0xC058B, // _CHAN_AFTER_BUTTON_Data_20180805170303.8.dat
                  0x937B3,0xF4607,
              };

 const int numtaps = sizeof(taps)/sizeof(taps[0]);
 int detect[sizeof(taps)/sizeof(taps[0])] = {0};
 uint64_t sh = 0;
 int total = 0;
 int idx = 0;
 while(1) {
  int n = fread(buf,1,sizeof(buf),f);
  int on = 0;
  if (n<1) break;
  for(int i=0;i<n;i++) {
      idx++;
   double in = (buf[i]&bitmask)?1.0:-1.0;
   // Filter it using simple FIR to get rid of some one-sample glitches around edges
   fil[0]=fil[1];fil[1]=fil[2];fil[2]=fil[3];fil[3]=fil[4];fil[4]=fil[5];fil[5]=in;
   double v=fil[0]*0.0280+fil[1]*0.1597+fil[2]*0.2779+fil[3]*0.3396+fil[4]*0.2779+fil[5]*0.1597+fil[6]*0.0280;
   double diff = v*old;
   old = v;
   dist++;
   if (diff<0 && dist>10) { // 10 = little bit more than one symbol length
    if (dist>100) { // long run of same bits between bursts
     int max = 0;
     int maxat = 0;
     for(int j=0;j<numtaps;j++) {
         if((detect[j] / (double)total) > .75)
      printf("%06X:%8d   %d %1.8f \n",taps[j],detect[j], total, detect[j] / (double)total); // detailed LFSR report
      if (detect[j]>max) {
       max=detect[j];
       maxat=j;
      }
     }
     sh = 0;
     total = 0;
     idx = 0;
     printf("(%d) found LFSR %06X bit %d 0x%lx",idx, taps[maxat],maxat&1, sh); // print best one to get bits
     printf("\n");
     for(int k=0;k<numtaps;k++) detect[k]=0;
    } else {
     int bit = (v>0)?1:0;
     //printf("%d",bit); // print raw demodulated bits if you want that

     sh <<= 1;
     sh |= bit;

     for(int j=0;j<numtaps;j++) {
      unsigned int b = sh&taps[j];
         b^=b>>16;b^=b>>8;b^=b>>4;b^=b>>2;b^=b>>1;b&=1; // parity
      if (!b) detect[j]++; // count zeroes
     }
        total++;
    }
    dist=0;
   }
  }
 }
 fclose(f);
 return 0;
}
