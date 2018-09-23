#include "types.h"

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
dataType getInflammability(int color) {
	switch(color) {
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
		case 0x273825: return 1;
		case 0x507166: return 2;
		case 0x67937f: return 3;
		case 0xa8d6c8: return 4;
		case 0x768d8a: return 5;
		default: return 0;
	}
}

int getColorForRendering(int identifier) {
	//"normalising"
	int id = (identifier < -6) ? -6 : identifier;
	switch(id) {
		case -6: return 0xffe808;
		case -5: return 0xffce00;
		case -4: return 0xff9a00;
		case -3: return 0xff5a00;
		case -2: return 0xff0000;
		case -1: return 0x373a24;
		default: return id;
	}
}
