#include <stdio.h>
#include <stdlib.h>
#include "decoderGG.h"
#include "decoderOA.h"
#include "decoderBD.h"
#include "decoderCBD.h"
#include "snc.h"

/* Definition of SNC decoder
 * It includes an allocated decoder context which contains encode
 * information (i.e., encode context) and miscellaneous structs that
 * are used during decoding. Different decoder types have different
 * decode context, so d_type is used as an indicator.
 */
struct snc_decoder {
    void   *dec_ctx;        // decoder context
    int    d_type;          // decoder type
};

struct snc_decoder *snc_create_decoder(struct snc_parameter sp, int d_type)
{
    struct snc_decoder *decoder = malloc(sizeof(struct snc_decoder));
    if (decoder == NULL)
        return NULL;

    decoder->d_type = d_type;

    switch (decoder->d_type) {
    case GG_DECODER:
        decoder->dec_ctx = create_dec_context_GG(sp);
        if (decoder->dec_ctx == NULL)
            goto failure;
        break;
    case OA_DECODER:
        decoder->dec_ctx = create_dec_context_OA(sp, 0);
        if (decoder->dec_ctx == NULL)
            goto failure;
        break;
    case BD_DECODER:
        decoder->dec_ctx = create_dec_context_BD(sp);
        if (decoder->dec_ctx == NULL)
            goto failure;
        break;
    case CBD_DECODER:
        decoder->dec_ctx = create_dec_context_CBD(sp);
        if (decoder->dec_ctx == NULL)
            goto failure;
        break;
    }
    return decoder;
failure:
    snc_free_decoder(decoder);
    return NULL;
}

void snc_process_packet(struct snc_decoder *decoder, struct snc_packet *pkt)
{
    switch (decoder->d_type) {
    case GG_DECODER:
        process_packet_GG(((struct decoding_context_GG *) decoder->dec_ctx), pkt);
        break;
    case OA_DECODER:
        process_packet_OA(((struct decoding_context_OA *) decoder->dec_ctx), pkt);
        break;
    case BD_DECODER:
        process_packet_BD(((struct decoding_context_BD *) decoder->dec_ctx), pkt);
        break;
    case CBD_DECODER:
        process_packet_CBD(((struct decoding_context_CBD *) decoder->dec_ctx), pkt);
        break;
    }
    return;
}

int snc_decoder_finished(struct snc_decoder *decoder)
{
    switch (decoder->d_type) {
    case GG_DECODER:
        return ((struct decoding_context_GG *) decoder->dec_ctx)->finished;
    case OA_DECODER:
        return ((struct decoding_context_OA *) decoder->dec_ctx)->finished;
    case BD_DECODER:
        return ((struct decoding_context_BD *) decoder->dec_ctx)->finished;
    case CBD_DECODER:
        return ((struct decoding_context_CBD *) decoder->dec_ctx)->finished;
    }
    return 0;
}

struct snc_context *snc_get_enc_context(struct snc_decoder *decoder)
{
    switch (decoder->d_type) {
    case GG_DECODER:
        return ((struct decoding_context_GG *) decoder->dec_ctx)->sc;
    case OA_DECODER:
        return ((struct decoding_context_OA *) decoder->dec_ctx)->sc;
    case BD_DECODER:
        return ((struct decoding_context_BD *) decoder->dec_ctx)->sc;
    case CBD_DECODER:
        return ((struct decoding_context_CBD *) decoder->dec_ctx)->sc;
    }
    return 0;
}

int snc_code_overhead(struct snc_decoder *decoder)
{
    switch (decoder->d_type) {
    case GG_DECODER:
        return ((struct decoding_context_GG *) decoder->dec_ctx)->overhead;
    case OA_DECODER:
        return ((struct decoding_context_OA *) decoder->dec_ctx)->overhead;
    case BD_DECODER:
        return ((struct decoding_context_BD *) decoder->dec_ctx)->overhead;
    case CBD_DECODER:
        return ((struct decoding_context_CBD *) decoder->dec_ctx)->overhead;
    }
    return 0;
}

long long snc_decode_cost(struct snc_decoder *decoder)
{
    switch (decoder->d_type) {
    case GG_DECODER:
        return ((struct decoding_context_GG *) decoder->dec_ctx)->operations;
    case OA_DECODER:
        return ((struct decoding_context_OA *) decoder->dec_ctx)->operations;
    case BD_DECODER:
        return ((struct decoding_context_BD *) decoder->dec_ctx)->operations;
    case CBD_DECODER:
        return ((struct decoding_context_CBD *) decoder->dec_ctx)->operations;
    }
    return 0;
}

void snc_free_decoder(struct snc_decoder *decoder)
{
    if (decoder == NULL)
        return;
    switch (decoder->d_type) {
    case GG_DECODER:
        free_dec_context_GG(((struct decoding_context_GG *) decoder->dec_ctx));
        break;
    case OA_DECODER:
        free_dec_context_OA(((struct decoding_context_OA *) decoder->dec_ctx));
        break;
    case BD_DECODER:
        free_dec_context_BD(((struct decoding_context_BD *) decoder->dec_ctx));
        break;
    case CBD_DECODER:
        free_dec_context_CBD(((struct decoding_context_CBD *) decoder->dec_ctx));
        break;
    }
    decoder->dec_ctx = NULL;
    free(decoder);
    decoder = NULL;
    return;
}


long snc_save_decoder_context(struct snc_decoder *decoder, const char *filepath)
{
    switch (decoder->d_type) {
    case GG_DECODER:
        return save_dec_context_GG((struct decoding_context_GG *) decoder->dec_ctx, filepath);
    case OA_DECODER:
        return save_dec_context_OA((struct decoding_context_OA *) decoder->dec_ctx, filepath);
    case BD_DECODER:
        return save_dec_context_BD((struct decoding_context_BD *) decoder->dec_ctx, filepath);
    case CBD_DECODER:
        return save_dec_context_CBD((struct decoding_context_CBD *) decoder->dec_ctx, filepath);
    }
}

struct snc_decoder *snc_restore_decoder(const char *filepath)
{
    struct snc_decoder *decoder;
    FILE *fp;
    if ((fp = fopen(filepath, "r")) == NULL) {
        fprintf(stderr, "Cannot open %s to load decoding context\n", filepath);
        return NULL;
    }
    // Check decoder context file type
    fseek(fp, sizeof(struct snc_metainfo), SEEK_SET);  // skip decoding_type field
    int d_type;
    fread(&d_type, sizeof(int), 1, fp);
    fclose(fp);
    if ((decoder = malloc(sizeof(struct snc_decoder))) == NULL)
        return NULL;
    switch (d_type) {
    case GG_DECODER:
        decoder->dec_ctx = restore_dec_context_GG(filepath);
        decoder->d_type = GG_DECODER;
        return decoder;
    case OA_DECODER:
        decoder->dec_ctx = restore_dec_context_OA(filepath);
        decoder->d_type = OA_DECODER;
        return decoder;
    case BD_DECODER:
        decoder->dec_ctx = restore_dec_context_BD(filepath);
        decoder->d_type = BD_DECODER;
        return decoder;
    case CBD_DECODER:
        decoder->dec_ctx = restore_dec_context_CBD(filepath);
        decoder->d_type = CBD_DECODER;
        return decoder;
    }
}
