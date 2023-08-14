# install keil MDK software (at least MDK-Essential)
https://www2.keil.com/mdk5

# open firmware_development\KL720\firmware\build\solution_kdp2_user_ex\sn72096_9x9\proj.uvmpw
execute 'bath build'

if sucessful, firmware binaries (fw_scpu.bin) will be put into res\firmware\KL720\

supported inference examples:
1) Generic RAW inference examples: kl720_demo_generic_data_inference, kl720_demo_generic_image_inference
2) Demo examples: kl720_demo_customize_inf_single_model, kl720_demo_generic_image_inference, kl720_demo_user_define_api
