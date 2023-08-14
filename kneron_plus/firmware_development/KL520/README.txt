# install keil MDK software (at least MDK-Essential)
https://www2.keil.com/mdk5

# open firmware_development\KL520\firmware\build\solution_kdp2_user_ex\sn52096\proj.uvmpw
execute 'bath build'

if sucessful, firmware binaries (fw_scpu.bin and fw_ncpu.bin) will be put into res\firmware\KL520\

supported inference examples:
1) Generic RAW inference examples: kl520_demo_generic_data_inference, kl520_demo_generic_image_inference
2) Demo examples: kl520_demo_customize_inf_single_model, kl520_demo_generic_image_inference, kl520_demo_user_define_api
