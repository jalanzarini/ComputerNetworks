#include "crc32.h"

#include <algorithm>

#define POLY_MAX_POW 32

const int poly32[] = {0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26, 32};
const int poly3[] = {0, 1, 3};

std::string crc32_remainder(std::string str){
	std::string binarystr;
	for(int i = 0; i < str.length(); i++) binarystr += ctob(str[i]);
	int ones = 0;
	for(int i = 0; i < binarystr.length(); i++) ones += binarystr[i] == '1';
	for(int i = 0; i < POLY_MAX_POW; i++) binarystr += "0";	
	for(int i = 0; ones > 0; i++){
		if(binarystr[i] != '1') continue;
		for(int j: poly32){
			int pos = i + (POLY_MAX_POW-j);
			char c = binarystr[pos]; 			
			binarystr[pos] = c == '0' ? '1': '0';
			if(pos < binarystr.length()-POLY_MAX_POW) ones += c == '0' ? 1: -1;	
		}
	}
	std::string remainder = binarystr.substr(binarystr.length()-POLY_MAX_POW, POLY_MAX_POW);
	for(int i = 0; i < remainder.length(); i++) remainder[i] = remainder[i] == '0' ? '1': '0';
	return remainder;
}

bool crc32_check(std::string str, std::string checksum){
	std::string binarystr;
	for(int i = 0; i < str.length(); i++) binarystr += ctob(str[i]);
	for(int i = 0; i < checksum.length(); i++) checksum[i] = checksum[i] == '0' ? '1': '0';
	binarystr += checksum;
	int ones = 0;
	for(int i = 0; i < binarystr.length(); i++) ones += binarystr[i] == '1';	
	for(int i = 0; ones > 0 && i < binarystr.length()-POLY_MAX_POW; i++){
		if(binarystr[i] != '1') continue;
		for(int j: poly32){
			int pos = i + (POLY_MAX_POW-j);
			char c = binarystr[pos]; 			
			binarystr[pos] = c == '0' ? '1': '0';
			ones += c == '0' ? 1: -1;	
		}
	}
	return ones == 0;
}

std::string ctob(char c){
	std::string binary;
	for(int i = 0; i < 8; i++){
		binary += c%2 == 1 ? "1": "0";
		c /= 2;
	}
	std::reverse(binary.begin(), binary.end());
	return binary;
}
