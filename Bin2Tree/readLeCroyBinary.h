#ifndef READ_LECROY_BINARY
#define READ_LECROY_BINARY 1
#include<stdio.h>
#include<vector>

#pragma pack(push, 1)

typedef struct{
    uint8_t descriptor_name[16];
    uint8_t template_name[16];
    int8_t comm_type1;
    int8_t comm_type2;
    int8_t comm_order1;
    int8_t comm_order2;
    int32_t wave_descriptor;
    int32_t user_text;
    int32_t res_desc1;
    int32_t trigtime_array;
    int32_t ris_time_array;
    int32_t res_array1;
    int32_t wave_array_1;
    int32_t wave_array_2;
    int32_t res_array2;
    int32_t res_array3;
    uint8_t instrument_name[16];
    int32_t instrument_number;
    uint8_t trace_label[16];
    int16_t reserved1;
    int16_t reserved2;
    int32_t wave_array_count;
    int32_t pnts_per_screen;
    int32_t first_valid_pnt;
    int32_t last_valid_pnt;
    int32_t first_point;
    int32_t sparsing_factor;
    int32_t segment_index;
    int32_t subarray_count;
    int32_t sweeps_per_acq;
    int16_t points_per_pair;
    int16_t pair_offset;
    float vertical_gain;
    float vertical_offset;
    float max_value;
    float min_value;
    int16_t nominal_bits;
    int16_t nom_subarray_count;
    float horizontal_interval;
    double horizontal_offset;
    double pixel_offset;
    uint8_t vertunit[48];
    uint8_t horunit[48];
    float horiz_uncertainty;
    double trigger_time_s;
    int8_t trigger_time_m;
    int8_t trigger_time_h;
    int8_t trigger_time_D;
    int8_t trigger_time_M;
    int16_t trigger_time_Y;
    int16_t trigger_time_r;
    float acq_duration;
    int16_t record_type;
    int16_t processing_done;
    int16_t reserved5;
    int16_t ris_sweeps;
    int16_t timebase;
    int16_t vert_coupling;
    float probe_att;
    int16_t fixed_vert_gain;
    int16_t bandwidth_limit;
    float vertical_vernier;
    float acq_vert_offset;
    int16_t wave_source;
} wavedesc;

#pragma pack(pop)

int readLeCroyBinary(FILE *, wavedesc *, std::vector<double> * , std::vector<int> *);

#endif
