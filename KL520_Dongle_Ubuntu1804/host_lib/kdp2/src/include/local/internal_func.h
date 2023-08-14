#ifndef __INTERNAL_FUNC_H__
#define __INTERNAL_FUNC_H__

#include "kdp2_struct.h"

int read_nef(char *nef_data, uint32_t nef_size, struct kdp2_metadata_s *metadata, struct kdp2_nef_info_s *nef_info);

#endif
