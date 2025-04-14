/*
 *   This program convert block data to spectra
 *
 	compilation 
 		$  g++  -o blk2gspect_v1730  blk2gspect_v1730-1_20201220.cpp


	usage
		$ ./blk2spect <filename> <outfile (w/o ext)> [ TOF ch_max ch_bunch ]
	examples:
		$ ./blk2spect filename.blk outfile 
		$ ./blk2spect filename.blk outfile TOF 60000 50
		$ 

 
 2020/02/22 for V730 DPP-PSD
  bkl2spect_v1730-0.cpp 
   bug fixed version
  blk2spect_v1730-1.cpp
   psd spectra and matrixes are also extracted
    
 
*
 */
// dagta 0x 12 34 56 78
// # define bigendian   // 87 56 34 12
// # define litendian   // 34 12 87 56

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

//#include <CAENDigitizer.h>

#include <iostream>
#define CHANNEL 16   // board input channel
// long long int は使わない方が良い

//#define DEBUG

//===================================
// definition of PSD 2D plot 
#define PSDMAT_PSD 512
#define PSDMAT_QL  512
#define SPECT_SIZE 65536

//===================================

struct TIME{
	int text ;
	int64_t ttag ;
	//double ttag;
};
struct EVENT{

	TIME t;
	int Qs, Ql, r;
	float psd;
	int flag_ch;

};
struct BOARD_AGG{
	int size;
	int64_t ttag;
	int counter;
	int mask_ch[CHANNEL];	// mask data of channel in a block buffer 
	int mask_sum;
	int mask; 
	int id;
	int bf;
	int32_t pettern;

};

struct GATE{
	long int Ql_l, Ql_h, PSD_l, PSD_h;
};

void print_four_char(unsigned char *c);
/* converte longendian */

// ------------------------------------------------
// utilities

void Read_Line(char *buff, int size, FILE *in)
{
    char string[256],*c;
    int i;
    do
    {
        c=fgets (buff,size,in);
        if (c==NULL)
        {
            buff[0]='\0';
            break;
        }

        i=sscanf(buff,"%s",string);
        if (i==EOF)
        {
            buff[0]='#';
            string[0]='#'  ;
        }
    } while (string[0]=='#');
}

// ------------------------------------------------

unsigned int lendian_conv2(unsigned char *c)
{
	unsigned int r;
	r=   c[1]*0x100 + c[0];

	return r;
}

unsigned long lendian_conv4(unsigned char *c)
{
	unsigned long r;
	r= c[3]*int64_t(0x1000000) + c[2]*0x10000+ c[1]*0x100 + c[0];
	return r;
}

unsigned long lendian_conv8(unsigned char *c)
{
	unsigned long long r,r2;

	r  = c[3]*int64_t(0x1000000) + c[2]*0x10000 + c[1]*0x100 + c[0] ;
	r2 = c[7]*int64_t(0x1000000) + c[6]*0x10000 + c[5]*0x100 + c[4] ;
	r = r2 *int64_t(0x100000000) +r;
	return r;
}

// arrange 32 bit lettle endian 
void lendian_arrange4(unsigned char *c)
{
	unsigned char C;
	C= c[0];
	c[0] = c[2];
	c[2] = C;
	C = c[1];
	c[1] = c[3];
	c[3] = C;
}

// routines for CAEN PSD blodk data analysis

// get event the simple data format w/o wave forms
// reviced for v1730 (2020/12/11)

int get_event(FILE *fp, EVENT *ev)
{
	unsigned char c[20];
	int n,i,j;
	int64_t ttag0;

	n=fread(&c, sizeof (char), 8, fp );
	if (n<8) return -1;

	/*for (j =0;j<4; j++)
	{
	  for (i =0;i<4; i++) printf ("%02x",c[3-i+j*4]);
	  printf (" ");
	}
 */
 	// trigger time tag 
	ttag0 = ev->t.ttag;
	// printf ("%x \t",c[3]);
	ev->t.ttag =   int64_t(0x1000000)* (c[3] & 0x7f)+ c[2]*0x10000 + c[1]*0x100 + c[0] ;

	// even odd channel (required for v1730)
	if ((c[3] & 0x80) == 0x80 )
	{ 
		ev->flag_ch = 1 ; // printf ("[1]%x ",c[3]);
	}
	else ev->flag_ch = 0 ; // printf ("[0]%x ",c[3]);

	if (ttag0 > ev->t.ttag ) 
	{ 
		ev->t.text ++ ;
		// print_four_char(&c[0]); 
		// std::cout << ev->t.ttag;
	}

	ev->Ql = c[7]*0x100 + c[6] ;
	ev->Qs = (c[5] & 0x7f)*0x100 + c[4] ;


	if (ev->Ql-ev->Qs <= 0) 
	{
		ev->psd = -1;
	}
	else   
	{
		ev->psd = float (ev->Ql-ev->Qs);
		ev->psd /= (ev->Ql);
	}

#ifdef DEBUG
	print_four_char(&c[0]);
	print_four_char(&c[4]);
#endif

	return 0 ;
}

void print_four_char(unsigned char *c)
{
	  int i;
	  for (i =0;i<4; i++) printf ("%02x",c[3-i]);
	  printf (" ");
}

// get four words (unsigned char; 0xFF)
int get_four_char(FILE *fp, unsigned char *c)
{
	int n;
	n=fread(c, sizeof (char), 4, fp );
	return n;

}
// board aggregation tag
// reviced for v1730 (2020/12/11)

int get_bd_agg_fmt(FILE *fp, unsigned char *c, BOARD_AGG *ba)
{
	int n, i, bit, mask, sum;
	n=fread(&c[4], sizeof (char), 12, fp );

	/* ba->size =  c[2]*0x10000 + c[1]*0x100 + c[0] ;  */
    ba->size = (c[0] & 0x0f) * int64_t(0x1000000) + c[1] * 0x10000 + c[2]* 100 + c[3];

	// get_four_char(fp, c);	

	//ba->ttag = int64_t(0x1000000)* c[3]+ c[2]*0x10000 + c[1]*0x100 + c[0] ;
	//ba->counter = ;
	
	// dual channel mask
	ba->mask =  c[4] ;
	mask = c[4];
	bit =1;
	sum =0;

	for (i =0; i<8; i++)
	{
		if (mask >>i & bit==1) 
		{
			ba->mask_ch[sum] =	i;
		//	printf ("B%d ", i);
			sum ++ ;
		}

	}
	 ba->mask_sum =	sum;
	// int id;


#ifdef DEBUG
	print_four_char(&c[0]);
	print_four_char(&c[4]);
	print_four_char(&c[CHANNEL]);
	print_four_char(&c[12]);
	printf ("  |  ");
#endif
	return ba->size;
}

int get_ch_agg_fmt(FILE *fp, unsigned char *c)
{

	int n,i,j;
	n=fread(&c[4], sizeof (char), 4, fp );
	int  size =  c[1]*0x100 + c[0] ;

#ifdef DEBUG
	print_four_char(&c[0]);
	print_four_char(&c[4]);

	printf ("; \n");
#endif

	return size;
}

int main(int argc,char **argv)
{
	FILE *fp, *fpl, *fps, *fppsd, *fg, *flist;
	int j,i,k;
	int ch_sel;
	// spectra
	long int ***spect_s;
	long int ***spect_l;
	long int ***spect_t;
	long int **event;
	long int tof_max;

	long int ***spect_psd;
	long int ***mat_psd;

	int tof_sw,  tof_bunch;

	// trigger time information 
	TIME ctime[CHANNEL];  // current

	/* バイナリ書き込み読み込みモードでファイルをオープン */
	char filename[82], outfile[82];
	char outfile_l[82];
	char outfile_s[82];
	char outfile_t[82];
	char outfile_psd[82];
	char outfile_asc[82];

	printf ("start sorting \n");
	
	// input and output files

	if (argc < 3)
	{
		printf ("command> $ ./blk2spect <filename> <outfile (w/o ext)> [ TOF ch_max ch_bunch ]");	
	}
	else
	{
		strcpy (filename, argv[1]);	
		//printf("%s \n", argv[1]);
		if((fp = fopen(filename, "rb")) == NULL )
		{
			printf("ファイルオープンエラー:%s \n", filename);
			exit(EXIT_FAILURE);
		}
		strcpy (outfile, argv[2]);	
	}

	if (argc > 3)
	{
		if (!strcmp (argv[3], "TOF") || !strcmp (argv[3], "tof") ) 
		{
			
			tof_sw =1; 
			tof_max =atoi(argv[4]);
			tof_bunch = atoi(argv[5]);
				printf ("Make TOF spectra (%d ch with bunch of %d)\n",tof_max, tof_bunch);
		}
		else 	tof_sw =0;
	}
	
//	printf ("Select channel number : ");
//	scanf ("%d", &ch_sel );
//	ch_sel = atoi(argv[2] );


// ---------------------------------------
//  read gate data

	int ngate, nchannel;
	int size =256;
	char buff[256];
	GATE **gate;
	
	if((fg = fopen("gate.txt", "rt")) == NULL )
	{
		printf("ファイルオープンエラー:%s \n", "gate.txt");
		exit(EXIT_FAILURE);
	}
	
	Read_Line(buff, size, fg);
	//printf ("%s\n ", buff);
	sscanf (buff, "%d%d", &nchannel, &ngate);
	//printf ("%d %d \n", nchannel, ngate);
	
	
    // memory allocation
	gate = (GATE **) calloc(nchannel, sizeof (GATE* ) );	
	//printf("*\n"); 
	if (gate == NULL ) 
	{
		printf ("memory allocation error: gate* \n");
		if (gate[i] == NULL )exit(EXIT_FAILURE) ;
	}
	for (i=0; i<nchannel; i++)
	{
		gate[i] = (GATE *) calloc(ngate, sizeof (GATE) );	
		//printf("-\n"); 
		
		if (gate[i] == NULL )
		{
			printf ("memory allocation error: gate[%d] \n", i); 
			exit(EXIT_FAILURE) ;
		}
	}
	
	//printf("X\n");
	// read gate condition
	for (i=0; i<nchannel; i++)
	{
		for (j=0; j<ngate; j++)
		{
	
			Read_Line(buff, size, fg);
			//printf ("%d %d : %s\n", i,j , buff);
			sscanf (buff, "%*d%d%d%d%d", 
				&gate[i][j].Ql_l, &gate[i][j].Ql_h, &gate[i][j].PSD_l, &gate[i][j].PSD_h);
		}

	}
	
	printf ("# gate parameter \n");
	printf ("# ch \t gate \t Ql_l \t Ql_h \t PSD_l \t PSD_h \n");
	for (i=0; i<nchannel; i++)
	{
		for (j=0; j<ngate; j++)
		{
			printf ("%d \t %d : %d \t %d \t %d \t %d \n",
				i,j,gate[i][j].Ql_l, gate[i][j].Ql_h, gate[i][j].PSD_l, gate[i][j].PSD_h);
		}

	}

	// exit(EXIT_SUCCESS);
// ----------------------------------------
// memory allocate for spectra

	printf ("memory allocation \n");
	event = (long int**)calloc (nchannel, sizeof (long int*));
	spect_s =  (long int ***)calloc(nchannel,sizeof(long int **));
	spect_l =  (long int ***)calloc(nchannel,sizeof(long int **));
	spect_psd =(long int ***)calloc(nchannel, sizeof(long int));

	mat_psd =  (long int ***)calloc(nchannel, sizeof(long int));
	if (tof_sw == 1) 
	{
			if ((spect_t=(long int ***)calloc(nchannel, sizeof(long int)))==NULL)
			{
				printf ("memory allocation error: for TOF spectra \n"); 
				exit(EXIT_FAILURE) ;
			}
	}
	if (spect_s ==NULL || spect_l == NULL || spect_psd == NULL || mat_psd == NULL   )
	{
			printf ("memory allocation error: for spectra \n"); 
			exit(EXIT_FAILURE) ;
	}
	

	for (i=0; i<nchannel; i++)
	{
		spect_s[i]=(long int **)calloc(ngate, sizeof(long int*));
		spect_l[i]=(long int **)calloc(ngate, sizeof(long int*));
		spect_psd[i]=(long int **)calloc(ngate, sizeof(long int*));

		mat_psd[i]=(long int **)calloc(ngate, sizeof(long int*));
		if (tof_sw == 1) 
				spect_t[i]=(long int **)calloc(ngate, sizeof(long int*));

		event[i] = (long int*)calloc (nchannel, sizeof (long int));

	}
 		
	for (i=0; i<nchannel; i++)
	{
		for (j=0; j<ngate; j++)
		{

			spect_s[i][j]=(long int *)calloc(sizeof(long int),65536);
			spect_l[i][j]=(long int *)calloc(sizeof(long int),65536);
			spect_psd[i][j]=(long int *)calloc(sizeof(long int),PSDMAT_PSD);

			mat_psd[i][j]=(long int *)calloc(sizeof(long int),PSDMAT_PSD*PSDMAT_QL);
			if (tof_sw == 1) 
					spect_t[i][j]=(long int *)calloc(sizeof(long int),tof_max);

			event[i][j] =0;

		}
		ctime[i].text=0;
		ctime[i].ttag=0;
	}

	// -------------------------------
	// read data
	// -------------------------------
	printf ("read data \n");
	unsigned char c[20];
	int n;
	int old_time=0;
	int interval=0;

	EVENT ev;
	BOARD_AGG ba;

	TIME tt;
	i=0;
	tt.text =  0;
	ev.t.text =0 ;
	ev.t.ttag =0 ;
	//n=fread(&c, sizeof (char), 16, fp );
	//printf ()F
	//int ch_sel = 0; // selected channel
	int ch_num;
	int block =0;   // header block number
	int tblock =0;  // total channel block number
	int cblock = 0; // channel block number
	int loop = 0;

	//output ascii list data
	sprintf (outfile_asc,"list_%s.asc",argv[2]);

	if((flist = fopen(outfile_asc, "wt")) == NULL ){
		printf("ファイルオープンエラー:%s \n", outfile_asc);
		exit(EXIT_FAILURE);
	}
	
	do {
		long int count=0;
		
		// progress dot
		if (loop ==1){
			 printf(".");
			fflush(stdout);
		}
		else if (loop == 99999) loop = 0;
		loop ++;
		
		// 
		get_four_char(fp, c);
//		print_four_char(c);
//		printf("\n");
		
		if ((c[3] & 0xf0) == 0xa0)
		{
			get_bd_agg_fmt( fp, c, &ba);
			//printf ("\n");


			block ++;
			tblock += ba.mask_sum;
			
			for (i =0; i < ba.mask_sum; i++)
			{
				get_four_char(fp, c);
				// count = get_ch_agg_fmt( fp, c);
				//print_four_char(c);
				//printf ("  %d words\n" , count);

				if (c[3] == 0x80)    // header ID of channel aggregate data
				{
					long int count = get_ch_agg_fmt( fp, c);
					//printf("%d ",count);

				//ch_sel = ba.mask_ch[i];
						
					for (j= 0; j<= count/2-2;j++)
					{
						get_event(fp, &ev);

						ch_sel = ba.mask_ch[i]*2+ev.flag_ch;
						ctime[ch_sel].text = ev.t.text;
						ctime[ch_sel].ttag = ev.t.ttag;
						
						int psd_ch =  ev.psd*PSDMAT_PSD;
						interval = ev.t.ttag - old_time;
						if(interval <0){
							interval = 0;
						}
						old_time = ev.t.ttag;
						fprintf(flist, "%d %d %d %d %d %f\n",ch_sel, ev.t.ttag, interval, ev.Ql, ev.Qs, ev.psd );
					}
				}
				else 
				{
					printf("somethng is wrong with channel aggregate data ...? \n");
				}
			}
			
		}
		else 
		{
			if (! feof(fp))
				printf("somethng is wrong with board data...? \n");
		}
		// reset c[]
		for (k=0; k<4; k++)
		{
			c[i] =0;
		}
		
		 i++;

	} while (! feof(fp) ); //&& block < 1);
	printf ("BLK: %d \n", block	);
	for (i=0; i<nchannel; i++)
	{
		printf("last record %d: \t %d \t %ld \n",i, ctime[i].text, ctime[i].ttag );
	}

/* ファイルクローズ */
	fclose (fp);
	fclose (flist);
	printf ("\n");

	return 0;

}

