/*************************************************************************
 > File Name: trim_bin.c
 > Author: joseph.wang
 > Email: 350187947@qq.com
 > Created Time: Thu Feb  6 22:43:54 2020
************************************************************************/
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOGI(format,...)  printf(format "\n", ##__VA_ARGS__)
#define LOGE(format,...)  printf(format "\n", ##__VA_ARGS__)


void show_usage()
{
    printf("trim_bin -i <input_file> -o <output_file> -s start_offset [-e end_offset]\n");
    printf("    start_offset and end_offset should be decimal\n");
}

#define BUF_SIZE_MAX    1024
static void trim_file(FILE *in, FILE *out, long size)
{
    uint8_t buf[BUF_SIZE_MAX];
    int ret, pkt_size, left_size = size;

    while (!feof(in) && left_size > 0) {
        pkt_size = left_size > BUF_SIZE_MAX ? BUF_SIZE_MAX : left_size;
        ret = fread(buf, sizeof(buf[0]), pkt_size, in);
        if (!ret)
            break;

        ret = fwrite(buf, sizeof(buf[0]), ret, out);
        if (!ret)
            break;
        left_size -= ret;
    }
}

int main(int argc, char *argv[])
{
	int opt;
    char *input_name = NULL, *output_name = NULL;
    long start_offset = ~0x0, end_offset = ~0x0, file_size = 0;
    FILE *infp = NULL, *outfp = NULL;

	while ((opt = getopt(argc, argv, "i:o:s:e:")) != -1) {
		switch (opt) {
        case 'i':
            input_name = optarg;
            break;
        case 'o':
            output_name = optarg;
            break;
        case 's':
            start_offset = atoi(optarg);
            break;
        case 'e':
            end_offset = atoi(optarg);
            break;
        default: /* '?' */
            show_usage();
            exit(EXIT_FAILURE);
		}
	}

    LOGI("param: input %s output %s start_offset %ld end_offset %ld",
            input_name, output_name, start_offset, end_offset);

    if (!input_name || !output_name || start_offset < 0) {
        LOGE("arguments error");
        show_usage();
        exit(EXIT_FAILURE);
    }

    /* open input file and output file */
    infp = fopen(input_name, "rb");
    outfp = fopen(output_name, "wb");
    if (!infp || !outfp) {
        LOGE("input file or output file open fail, infp %p, outfp %p",
                infp, outfp);
        goto error;
    }

    /* get input file size */
    fseek(infp, 0L, SEEK_END);
    file_size = ftell(infp);
    if (file_size <= start_offset) {
        LOGE("file size is not large enough, start_offset %ld should less than file size %ld",
                start_offset, file_size);
        goto error;
    }
    end_offset = end_offset < 0 ? file_size : end_offset;
    end_offset = end_offset > file_size ? file_size : end_offset;
    if (end_offset <= start_offset) {
        LOGE("offset error, start_offset %ld should less than end_offset %ld",
                start_offset, end_offset);
        goto error;
    }

    /* seek input file offset */
    fseek(infp, start_offset, SEEK_SET);

    /* read input file context and write into output file */
    trim_file(infp, outfp, end_offset - start_offset);

    /* close input file and output file*/
    if (infp) fclose(infp);
    if (outfp) fclose(outfp);

    LOGI("trim done!");

    return 0;
error:
    if (infp) fclose(infp);
    if (outfp) fclose(outfp);

    exit(EXIT_FAILURE);
}

