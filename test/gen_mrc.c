
#include <stdio.h>
#include <mrc.h>

/*
 *
 * Test file for libmrc
 * INPUT: file containing references (\n) seperated
 * OUTPUT: miss ratio curve for the trace
 *
 * Usage: ./gen_mrc <input_file>
 * Outputs <input_file>.mrc
*/


int main( int argc, char **argv ) 
{
    FILE *in = fopen( argv[1], "r");
    if (!in)
        fprintf(stderr,"error opening %s\n",argv[1]);
    
    char out_name[128];
    sprintf(out_name,"%s.mrc",argv[1]);

    FILE *out = fopen( out_name, "w" );

    if (!out)
        fprintf(stderr,"error opening %s\n",out_name);
    
    
    mrc_t *a_mrc = init_mrc(atoi(argv[2]),atoi(argv[3]));
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line,&len,in)) != -1)
    {
        uint64_t reference = atoi(line);
        take_sample(a_mrc,reference);
    }
    if (line)
        free(line);
    fclose(in);

    solve_mrc(a_mrc);

    fprintf(out,"mr,size\n");
    uint64_t c = 1;
    while (c < a_mrc->mrc_size)
    {
        uint64_t csize = c*a_mrc->mrc_interval;
        fprintf(out,"%f,%llu\n",a_mrc->mrc[c],csize);
        c++;
    }

    fclose(out);
    delete_mrc(a_mrc);

}
