#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	if(argc <= 1) {
		printf("Too few arguments\n");
		exit(1);
	} else if(argc >= 5) {
		printf("Too many arguments\n");
		exit(1);
	}
	
	FILE* fphzk = NULL;
	FILE* font = NULL;
	char *file;
	int i, k, offset, count, flag;
	unsigned char buffer[32];
	unsigned char word1[] = "欢迎来到矩阵设计制作：邓飞郝梧桐唐宇瀚王弘儒按空格键继续";
	unsigned char word2[] = "乙八勹匕甘瓜禾卜刀儿二匚几冂力凵人入十厶匸又艹屮巛川寸大飞干工弓廾己彐彑巾口马门宀女山尸士巳土囗兀夕小幺弋尢夂子贝比长车歹斗厄方风父戈卝户火旡见斤耂毛木肀牛爿片攴攵气欠犬日氏手殳水瓦尣王韦文毋心牙爻曰月支止爪臣虫而耳缶艮虍臼米齐肉色舌豆谷見角克里卤麦身豕辛";

	if(strcmp(argv[1], "s") == 0)
		file = "hzk16s";
	else if(strcmp(argv[1], "f") == 0)
		file = "hzk16f";
	else if(strcmp(argv[1], "h") == 0)
		file = "hzk16h";
	else if(strcmp(argv[1], "k") == 0)
		file = "hzk16k";
	else if(strcmp(argv[1], "y") == 0)
		file = "hzk16y";
	else if(strcmp(argv[1], "x") == 0)
		file = "hzk16x";
	else if(strcmp(argv[1], "v") == 0)
		file = "hzk16v";
	else if(strcmp(argv[1], "c") == 0)
		file = "hzk16c";
		
	if(strcmp(argv[3], "cn") == 0)
		flag = 1;
	else if(strcmp(argv[3], "jp") == 0)
		flag = 2;
	else {
		printf("Wrong country code");
		exit(1);
	}
		
	printf("font 1: %s\n", file);
	fphzk = fopen(file, "rb");
	
	font = fopen("/workdir/mp3/student-distrib/cn_font.h", "w");
	
	if(fphzk == NULL || font == NULL) {
		fprintf(stderr, "file open error\n");
		return 1;
	}
	
	fprintf(font, "/* Created by:\n");
	fprintf(font, " *\t\tFei Deng, Wutong Hao, Yuhan Tang, Hongru Wang\n");
	fprintf(font, " *\t\tECE 391, 2014 Spring\n");
	fprintf(font, " *\t\tGroup 27\n");
	fprintf(font, " *\n * Chinese font data */\n\n");
	fprintf(font, "#ifndef _CN_FONT_H\n");
	fprintf(font, "#define _CN_FONT_H\n\n");
	fprintf(font, "#include \"types.h\"\n\n\n");
	fprintf(font, "#define CN_CHAR_NUM\t\t%d\n", (sizeof(word1) - 1) / 2);
	fprintf(font, "#define CN_FONT_NUM1\t(CN_CHAR_NUM * 2)\n");
	
	if(flag == 1)
		fprintf(font, "#define CN_FONT_NUM2\t256\n");
	else if(flag == 2)
		fprintf(font, "#define CN_FONT_NUM2\t166\n");
	
	fprintf(font, "#define CN_FONT_HEIGHT\t16\n\n");
	fprintf(font, "uint8_t cn_font_data1[CN_FONT_NUM1][CN_FONT_HEIGHT] = {\n");
	
	i = 0;
	while(i < sizeof(word1) - 1) {
	
		offset = (94 * (unsigned int)(word1[i] - 0xA0 - 1) + (word1[i + 1] - 0xA0 - 1)) * 32;
		i += 2;
		fseek(fphzk, offset, SEEK_SET);
		fread(buffer, 1, 32, fphzk);
		
		fprintf(font ,"\t{");
		
		count = 0;
		for(k = 0; k < 32; k += 2) {
			count++;
			fprintf(font, "0x%02x", buffer[k]);
			
			if(count < 8)
				fprintf(font, ", ");
			else if(count > 8 && count < 16)
				fprintf(font, ", ");
			
			if(count == 8)
				fprintf(font, ",\n\t ");
		}
		fprintf(font, "},\n\t{");
		
		count = 0;
		for(k = 1; k < 32; k += 2) {
			count++;
			fprintf(font, "0x%02x", buffer[k]);
			
			if(count < 8)
				fprintf(font, ", ");
			else if(count > 8 && count < 16)
				fprintf(font, ", ");
			
			if(count == 8)
				fprintf(font, ",\n\t ");
		}
		
		if(i < sizeof(word1) - 1)
			fprintf(font, "},\n\n");
		else
			fprintf(font, "}\n};\n\n");
	}
	
	fclose(fphzk);
	
	if(strcmp(argv[2], "s") == 0)
		file = "hzk16s";
	else if(strcmp(argv[2], "f") == 0)
		file = "hzk16f";
	else if(strcmp(argv[2], "h") == 0)
		file = "hzk16h";
	else if(strcmp(argv[2], "k") == 0)
		file = "hzk16k";
	else if(strcmp(argv[2], "y") == 0)
		file = "hzk16y";
	else if(strcmp(argv[2], "x") == 0)
		file = "hzk16x";
	else if(strcmp(argv[2], "v") == 0)
		file = "hzk16v";
	else if(strcmp(argv[2], "c") == 0)
		file = "hzk16c";

	printf("font 2: %s\n", file);
	fphzk = fopen(file, "rb");
	
	fprintf(font, "uint8_t cn_font_data2[CN_FONT_NUM2][CN_FONT_HEIGHT] = {\n");
	i = 0;
	
	if(flag == 1) {
		while(i < sizeof(word2) - 1) {
		
			offset = (94 * (unsigned int)(word2[i] - 0xA0 - 1) + (word2[i + 1] - 0xA0 - 1)) * 32;
			i += 2;
			fseek(fphzk, offset, SEEK_SET);
			fread(buffer, 1, 32, fphzk);
			
			fprintf(font ,"\t{");
			
			count = 0;
			for(k = 0; k < 32; k += 2) {
				count++;
				fprintf(font, "0x%02x", buffer[k]);
				
				if(count < 8)
					fprintf(font, ", ");
				else if(count > 8 && count < 16)
					fprintf(font, ", ");
				
				if(count == 8)
					fprintf(font, ",\n\t ");
			}
			fprintf(font, "},\n\t{");
			
			count = 0;
			for(k = 1; k < 32; k += 2) {
				count++;
				fprintf(font, "0x%02x", buffer[k]);
				
				if(count < 8)
					fprintf(font, ", ");
				else if(count > 8 && count < 16)
					fprintf(font, ", ");
				
				if(count == 8)
					fprintf(font, ",\n\t ");
			}
			
			if(i < sizeof(word2) - 1)
				fprintf(font, "},\n\n");
			else
				fprintf(font, "}\n};\n\n");
		}
	} else if(flag == 2) {
		
		int qu_ma = 3, wei_ma = 0;
		
		while(i < 83) {
		
			offset = (94 * (unsigned int)qu_ma + (unsigned int)wei_ma) * 32;
			
			i++;
			if(wei_ma >= 93) {
				wei_ma = 0;
				qu_ma++;
			} else
				wei_ma++;
			
			fseek(fphzk, offset, SEEK_SET);
			fread(buffer, 1, 32, fphzk);
			
			fprintf(font ,"\t{");
			
			count = 0;
			for(k = 0; k < 32; k += 2) {
				count++;
				fprintf(font, "0x%02x", buffer[k]);
				
				if(count < 8)
					fprintf(font, ", ");
				else if(count > 8 && count < 16)
					fprintf(font, ", ");
				
				if(count == 8)
					fprintf(font, ",\n\t ");
			}
			fprintf(font, "},\n\t{");
			
			count = 0;
			for(k = 1; k < 32; k += 2) {
				count++;
				fprintf(font, "0x%02x", buffer[k]);
				
				if(count < 8)
					fprintf(font, ", ");
				else if(count > 8 && count < 16)
					fprintf(font, ", ");
				
				if(count == 8)
					fprintf(font, ",\n\t ");
			}
			
			if(i < 83)
				fprintf(font, "},\n\n");
			else
				fprintf(font, "}\n};\n\n");
		}
	}
	
	fprintf(font, "#endif /* _CN_FONT_H */\n");

	fclose(fphzk);
	fclose(font);
	fphzk = NULL;
	
	return 0;
}
