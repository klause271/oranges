/***************************calculator*********************************/
#include "stdio.h"

void main()
{
	int result;
	char first_num[8];
	char second_num[8];
	char operator[2];

	printf("The first number:");
	int r1 = read(0, first_num, 8);
	first_num[r1] = 0;
	int a = ctoi(first_num);

	printf("The second number:");
	int r2 = read(0, second_num, 8);
	second_num[r2] = 0;
	int b = ctoi(second_num);

	printf("The operator:");
	read(0, operator, 2);

	switch(operator[0]){
	case '+':
		result = a + b;
		break;
	case '-':
		result = a - b;
		break;
	case '*':
		result = a * b;
		break;
	case '/':
		if(b){
			result = a / b;
		} else {
			printf("[ERROR]DIV / 0!\n");
			return;
		}
		break;	
	default:
		printf("error input!\n");
	}

	printf("%d %c %d = %d\n", a, operator[0], b, result);

}

int ctoi(const char *s)
{
	int num = 0;
	int i = 0;
	char ch;
	while(s[i]) {
		ch = s[i];
		i++;
		if (ch < '0' || ch > '9')
			break;
		num = num * 10 + (ch - '0');
	}
	return num;
}
