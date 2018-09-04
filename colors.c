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
		/*case 0x273825: return 2;
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
		case 0x13361e: return 1;
		case 0x112d12: return 1;
		case 0x112c18: return 1;
		case 0x1a4123: return 1;
		case 0x1a3c24: return 1;
		case 0x1a2f1e: return 1;
		case 0x223a1f: return 1;
		case 0x1b351f: return 1;
		case 0x2c4c1d: return 2;
		case 0x193614: return 1;
		case 0x2b462f: return 1;
		case 0x234824: return 3;
		case 0x1e3d20: return 3;
		case 0x234326: return 3;
		case 0x224225: return 3;
		case 0x2c4c2e: return 3;
		case 0x253f28: return 3;
		case 0x23462d: return 2;
		case 0x2a4724: return 4;
		case 0x27422b: return 5;
		case 0x2f4c29: return 2;
		case 0x2e4b23: return 3;
		case 0x334928: return 3;
		case 0x29492b: return 1;
		case 0x223c26: return 2;
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
		case 0x646f5e: return 1;
		case 0x2e462a: return 3;
		case 0x17361a: return 1;
		case 0x1b3a1e: return 1;
		case 0x334b2e: return 0;
		case 0x394f2d: return 4; //or 0
		case 0x365034: return 0;
		case 0x36472b: return 0;
		case 0x3f4e2e: return 0;
		case 0x263d2d: return 0;
		case 0x293c27: return 0;*/
		case 0x061a11: return 0;
		case 0x0b1c1d: return 0;
		case 0x0a1f0d: return 0;
		case 0x142821: return 0;
		case 0x193614: return 1;
		case 0x17361a: return 1;
		case 0x19351f: return 1;
		case 0x1b3a1e: return 3;
		case 0x223a1f: return 2;
		case 0x1a3c24: return 3;
		case 0x203b24: return 3;
		case 0x1e3d20: return 3;
		case 0x2a3a20: return 3;
		case 0x283d2a: return 3;
		case 0x234114: return 3;
		case 0x2b3c2f: return 0;
		case 0x1f4129: return 3;
		case 0x224124: return 3;
		case 0x254029: return 3;
		case 0x284024: return 1;
		case 0x2f4025: return 0;
		case 0x28462e: return 1;
		case 0x27472a: return 3;
		case 0x2b4927: return 3;
		case 0x314634: return 0;
		case 0x30482a: return 3;
		case 0x3c4435: return 0;
		case 0x36472b: return 0;
		case 0x2c4c1d: return 3;
		case 0x2e4b23: return 4;
		case 0x35512c: return 4;
		case 0x375035: return 3;
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
		case -1: return 0x373a24;
		//case -1: return 0;
		default: return identifier;
	}
}
