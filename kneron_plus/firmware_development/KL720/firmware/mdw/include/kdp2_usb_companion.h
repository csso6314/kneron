#pragma once

int kdp2_usb_companion_init(void);

int kdp2_cmd_handler_initialize(void);
int kdp2_cmd_handle_kp_command(uint32_t command_header_buf);     // for new KDP2 commands
int kdp2_cmd_handle_legend_kdp_command(uint32_t command_buffer); // for old KDP commands

int kdp2_usb_log_initialize(void);
