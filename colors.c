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
		case 0x273825: return 2;
		case 0x1c2a19: return 2;
		case 0x3b5051: return 2;
		case 0x0b107c: return 2;
		case 0x507166: return 3;
		case 0x768d8a: return 5;
		case 0x67937f: return 4;
		case 0x8ab19f: return 4;
		case 0xa8d6c8: return 5;
		case 0x10180b: return 1;
		case 0x0b1b09: return 1;
		case 0x142110: return 1;
		case 0x192015: return 1;
		case 0x28211a: return 4;
		case 0x232816: return 1;
		case 0x202820: return 1;
		case 0x262811: return 4;
		/*case 0x21291b: return 1;
		case 0x2c3129: return 1;
		case 0x233323: return 1;
		case 0x26331f: return 1;
		case 0x273229: return 1;
		case 0x2a3224: return 2;
		case 0x313b31: return 1;
		case 0x333b2d: return 1;
		case 0x353b28: return 1;
		case 0x2b3d22: return 1;
		case 0x2c3c2c: return 1;
		case 0x2e3c28: return 1;
		case 0x354536: return 1;
		case 0x3a443a: return 1;
		case 0x3c4435: return 2;
		case 0x3a4732: return 1;
		case 0x424e40: return 1;
		case 0x475845: return 1;
		case 0x536350: return 1;
		case 0x646f5e: return 1;*/
		case 0x386261: return 1;
		case 0x434e52: return 1;
		case 0x5b897f: return 1;
		case 0x74875a: return 1;
		case 0x3e5855: return 2;
		case 0x709079: return 1;
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
