//scale: 0 - lush, 5 - dry
int getTreeColor(int identifier){
	switch(identifier) {
		case 1: return 0x273825;
		case 2: return 0x507166;
		case 3: return 0x67937f;
		case 4: return 0xa8d6c8;
		case 5: return 0x768d8a;
		default: return identifier;
	}
}

//0 not burnable, 5 highly inflammable
int getInflammability(int color) {
	switch(color) {
		case 0x273825: return 1;
		case 0x1c2a19: return 1;
		case 0x3b5051: return 1;
		case 0x0b107c: return 1;
		case 0x507166: return 2;
		case 0x768d8a: return 5;
		case 0x67937f: return 3;
		case 0x8ab19f: return 3;
		case 0xa8d6c8: return 4;
		default: return 0;
	}
}

int getColorForRendering(int identifier) {
	switch(identifier) {
		case -6: return 0xffe808;
		case -5: return 0xffce00;
		case -4: return 0xff9a00;
		case -3: return 0xff5a00;
		case -2: return 0xff0000;
		case -1: return 0x421600;
		default: return identifier;
	}
}