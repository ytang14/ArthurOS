unsigned short rtc_freq = 32;

unsigned char note_data[] = {
	0x33, 0x31, 0x30, 0x31, 0x34, 0x00, 0x36,
	0x34, 0x33, 0x34, 0x38, 0x00, 0x39, 0x38, 0x37,
	0x38, 0x3f, 0x3d, 0x3c, 0x3d, 0x3f, 0x3d, 0x3c,
	0x3d, 0x40, 0x3d, 0x40, 0x3f, 0x3d, 0x3b, 0x3d,
	0x3f, 0x3d, 0x3b, 0x3d, 0x3f, 0x3d, 0x3b, 0x3a,
	0x38,
	0x33, 0x31, 0x30, 0x31, 0x34, 0x00, 0x36,
	0x34, 0x33, 0x34, 0x38, 0x00, 0x39, 0x38, 0x37,
	0x38, 0x3f, 0x3d, 0x3c, 0x3d, 0x3f, 0x3d, 0x3c,
	0x3d, 0x40, 0x3d, 0x40, 0x3f, 0x3d, 0x3b, 0x3d,
	0x3f, 0x3d, 0x3b, 0x3d, 0x3f, 0x3d, 0x3b, 0x3a,
	0x38,
	56,57,
	59,59,61,59,57,56,
	54,54,56,57,
	59,59,61,59,57,56,
	54,52,54,
	56,56,57,56,54,52,
	51,51,52,54,
	56,56,57,56,54,52,
	51,51,49,48,49,
	52,0,54,52,51,52,
	56,0,57,56,55,56,
	63,61,60,61,63,61,60,61,
	64,61,63,
	64,63,61,60,
	61,56,57,54,
	52,51,52,51,49,51,
	49,
	56,57,
	59,59,61,59,57,56,
	54,54,56,57,
	59,59,61,59,57,56,
	54,52,54,
	56,56,57,56,54,52,
	51,51,52,54,
	56,56,57,56,54,52,
	51,51,49,48,49,
	52,0,54,52,51,52,
	56,0,57,56,55,56,
	63,61,60,61,63,61,60,61,
	64,61,63,
	64,63,61,60,
	61,56,57,54,
	52,51,52,51,49,51,
	49,
	61,63,
	65,61,63,
	65,63,61,60,
	58,60,61,63,
	60,56,61,63,
	65,61,63,
	65,63,61,60,
	58,63,60,56,
	61,
	61,63,
	65,61,63,
	65,63,61,60,
	58,60,61,63,
	60,56,61,63,
	65,61,63,
	65,63,61,60,
	58,63,60,56,
	61,
	65,66,65,63,
	61,63,61,60,58,61,60,58,
	57,58,60,57,53,55,57,53,
	58,57,58,60,61,60,61,63,
	65,64,65,64,65,66,65,63,
	61,63,61,60,58,61,60,58,
	56,58,60,56,53,55,56,53,
	55,56,58,55,52,53,55,52,
	53,
	65,66,65,63,
	61,63,61,60,58,61,60,58,
	57,58,60,57,53,55,57,53,
	58,57,58,60,61,60,61,63,
	65,64,65,64,65,66,65,63,
	61,63,61,60,58,61,60,58,
	56,58,60,56,53,55,56,53,
	55,56,58,55,52,53,55,52,
	53,
	56,54,53,51,
	49,51,53,54,56,58,60,61,
	61,60,58,56,56,54,53,51,
	49,51,53,54,56,58,60,61,
	62,63,56,54,53,51,
	49,51,53,54,56,58,60,61,
	61,60,58,56,56,54,53,51,
	53,56,49,53,51,54,48,51,
	49,65,66,65,63,
	61,63,61,60,58,61,60,58,
	57,58,60,57,53,55,57,53,
	58,57,58,60,61,60,61,63,
	65,64,65,64,65,64,65,62,
	66,65,66,65,66,65,66,65,
	66,65,63,61,60,61,63,60,
	61,63,65,61,57,58,60,57,
	58,
	56,54,53,51,
	49,51,53,54,56,58,60,61,
	61,60,58,56,56,54,53,51,
	49,51,53,54,56,58,60,61,
	62,63,56,54,53,51,
	49,51,53,54,56,58,60,61,
	61,60,58,56,56,54,53,51,
	53,56,49,53,51,54,48,51,
	49,65,66,65,63,
	61,63,61,60,58,61,60,58,
	57,58,60,57,53,55,57,53,
	58,57,58,60,61,60,61,63,
	65,64,65,64,65,64,65,62,
	66,65,66,65,66,65,66,65,
	66,65,63,61,60,61,63,60,
	61,63,65,61,57,58,60,57,
	58,
	61,63,
	65,61,63,
	65,63,61,60,
	58,60,61,63,
	60,56,61,63,
	65,61,63,
	65,63,61,60,
	58,63,60,56,
	61,
	61,63,
	65,61,63,
	65,63,61,60,
	58,60,61,63,
	60,56,61,63,
	65,61,63,
	65,63,61,60,
	58,63,60,56,
	61,
	0x33, 0x31, 0x30, 0x31, 0x34, 0x00, 0x36,
	0x34, 0x33, 0x34, 0x38, 0x00, 0x39, 0x38, 0x37,
	0x38, 0x3f, 0x3d, 0x3c, 0x3d, 0x3f, 0x3d, 0x3c,
	0x3d, 0x40, 0x3d, 0x40, 0x3f, 0x3d, 0x3b, 0x3d,
	0x3f, 0x3d, 0x3b, 0x3d, 0x3f, 0x3d, 0x3b, 0x3a,
	0x38,
	0x33, 0x31, 0x30, 0x31, 0x34, 0x00, 0x36,
	0x34, 0x33, 0x34, 0x38, 0x00, 0x39, 0x38, 0x37,
	0x38, 0x3f, 0x3d, 0x3c, 0x3d, 0x3f, 0x3d, 0x3c,
	0x3d, 0x40, 0x3d, 0x40, 0x3f, 0x3d, 0x3b, 0x3d,
	0x3f, 0x3d, 0x3b, 0x3d, 0x3f, 0x3d, 0x3b, 0x3a,
	0x38,
	56,57,
	59,59,61,59,57,56,
	54,54,56,57,
	59,59,61,59,57,56,
	54,52,54,
	56,56,57,56,54,52,
	51,51,52,54,
	56,56,57,56,54,52,
	51,51,49,48,49,
	52,0,54,52,51,52,
	56,0,57,56,55,56,
	63,61,60,61,63,61,60,61,
	64,61,63,
	64,63,61,60,
	61,56,57,54,
	52,51,52,51,49,51,
	49,
	56,57,
	59,59,61,59,57,56,
	54,54,56,57,
	59,59,61,59,57,56,
	54,52,54,
	56,56,57,56,54,52,
	51,51,52,54,
	56,56,57,56,54,52,
	51,51,49,48,49,
	52,0,54,52,51,52,
	56,0,57,56,55,56,
	63,61,60,61,63,61,60,61,
	64,61,63,
	64,63,61,60,
	61,56,57,54,
	52,51,52,51,49,51,
	49,
	49,61,51,63,
	53,65,0,49,61,51,63,
	53,65,51,63,49,61,48,60,
	46,58,48,60,49,61,51,63,
	48,60,44,56,49,61,51,63,
	53,65,0,49,61,51,63,
	53,65,51,63,49,61,48,60,
	46,58,51,63,48,60,44,56,
	61,
	49,61,51,63,
	53,65,0,49,61,51,63,
	53,65,51,63,49,61,48,60,
	46,58,48,60,49,61,51,63,
	48,60,44,56,49,61,51,63,
	53,65,0,49,61,51,63,
	53,65,51,63,49,61,48,60,
	46,58,51,63,48,60,44,56,
	61,65,65,65,
	65,
	65,
	66,65,63,65,66,65,63,65,
	66,
	
	66,65,65,66,65,65,66,65,65,66,65,65,
	63,63,68,
	65,
	65,
	66,65,63,65,66,65,63,65,
	66,
	65,
	65,63,63,65,63,63,65,63,63,65,63,63,
	61,65,65,65,
	65,
	65,
	66,65,63,65,66,65,63,65,
	66,
	66,65,65,66,65,65,66,65,65,66,65,65,
	63,63,68,
	65,
	65,
	66,65,63,65,66,65,63,65,
	66,
	65,
	65,63,63,65,63,63,65,63,63,65,63,63,
	61,61,65,
	61,61,68,
	61,61,65,
	61,65,61,68,
	61,61,
	61,0,
};

unsigned char speed_data[] = {
	0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x03,
	0x03, 0x03, 0x03, 0x04, 0x04, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05,
	0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x03,
	0x03, 0x03, 0x03, 0x04, 0x04, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05,
	4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,3,3,3,3,
	4,4,3,3,3,3,
	4,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,3,3,3,2,2,
	5,
	4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,3,3,3,3,
	4,4,3,3,3,3,
	4,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,3,3,3,2,2,
	5,
	4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	4,4,4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,
	4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	4,4,4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,
	3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,
	3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,
	3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	4,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,
	3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	4,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,
	4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	4,4,4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,
	4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	4,4,4,4,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,
	0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x03,
	0x03, 0x03, 0x03, 0x04, 0x04, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05,
	0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x03,
	0x03, 0x03, 0x03, 0x04, 0x04, 0x03, 0x03, 0x03,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x03, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x05,
	4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,3,3,3,3,
	4,4,3,3,3,3,
	4,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,3,3,3,2,2,
	5,
	4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,4,4,
	4,4,3,3,3,3,
	4,4,4,4,
	4,4,3,3,3,3,
	5,3,3,3,3,
	4,4,3,3,3,3,
	4,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,4,4,
	4,4,4,4,
	4,4,4,4,
	5,3,3,3,2,2,
	5,
	3,3,3,3,
	3,3,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,
	3,3,3,3,
	3,3,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,4,3,3,3,3,
	3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,
	5,4,3,3,
	6,
	6,
	3,3,3,3,3,3,3,3,
	6,
	
	2,2,3,2,2,3,2,2,3,2,2,3,
	5,4,4,
	6,
	6,
	3,3,3,3,3,3,3,3,
	6,
	6,
	2,2,3,2,2,3,2,2,3,2,2,3,
	5,4,3,3,
	6,
	6,
	3,3,3,3,3,3,3,3,
	6,
	2,2,3,2,2,3,2,2,3,2,2,3,
	5,4,4,
	6,
	6,
	3,3,3,3,3,3,3,3,
	6,
	6,
	2,2,3,2,2,3,2,2,3,2,2,3,
	5,4,4,
	5,4,4,
	5,4,4,
	4,4,4,4,
	5,5,
	5,5,
};
