# Runs the blk2gspect_v1730_r1.exe for each of the binary data
# usage:
#   $ awk -f blk2gspct.awk -v RUN=number

BEGIN{

	DIR= "spect"
	LIST="list"
	EXE="progs/src"

	i = RUN
	for (j=1 ; j<=20; j++)
	{
				blkfile = sprintf ("%s/Run%04d-%04d_Bd0.blk", LIST, i, j); # example list/Run1710-0001_Bd0.blk
				outfile = sprintf ("$s/Run%04d-%04d", DIR, i, j); # example spect/Run1710-0001
				outfile_o = sprintf ("%04d-%02d", i, j); # example 1710-01


				if (getline < blkfile !=1) # if data file does not exist
				{
					print "no block file , end of program ", blkfile;
					exit
				}


				#print outfile_o, blkfile
				
				{
					close (blkfile)
					close (outfile)
					# command = sprintf ("%s/blk2spect.exe %s  %s  TOF 20000 500 ", blkfile, outfile);
					command = sprintf ("%s/blk2gspect_v1730_r1.exe %s  %s TOF 100000 100 ", EXE, blkfile, outfile_o);
					print command
					system(command)
				#	print command
				#	close (blkfile);
				}

	}
	exit;

}
{}
END{}

