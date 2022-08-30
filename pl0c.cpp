/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//LIBRERIAS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//USING NAMESPACE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//STRUCTS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VARIBALES GLOBALES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int linea = 0;		//para saber cuantas lineas tiene el codigo fuente
static char lectura;		//donde se guardan los caracteres que se leen del archivo

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CONSTANTES GLOBALES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define	PUNTO      		"."
#define	IDENT      		"ident"
#define	NUMERO     		"num"
#define	STRING     		"string"
#define	CONSTANTE  		"const"
#define	VARIABLE   		"var"
#define	PROCEDURE  		"procedure"
#define	CALL       		"call"
#define	BEGIN      		"begin"
#define	END        		"end"
#define	IF         		"if"
#define	THEN       		"then"
#define	WHILE      		"while"
#define	DO         		"do"
#define	ODD        		"odd"
#define	ESCRIBIR   		"write"
#define	ESCRIBIR_LN		"writeln"
#define	LEER_LN    		"readln"
#define	IGUAL      		"="
#define	DISTINTO   		"<>"
#define	MAYOR      		">"
#define	MENOR      		"<"
#define	MAYOR_IGUAL		">="
#define	MENOR_IGUAL		"<="
#define	SUMA       		"+"
#define	RESTA      		"-"
#define	MULTIPLICACION	"*"
#define	DIVISION      	"/"
#define	PARENTESIS_L  	"("
#define	PARENTESIS_R  	")"
#define	ASIGNACION    	":="
#define	PUNTO_COMA    	";"
#define	COMA			","
#define	COMILLA_SIMPLE	"'"
#define	COMILLA_DOBLE	"\""
#define	FIN_PROGRAMA  	"EOF"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROTOTIPOS DE FUNCIONES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void error(string);				//manejo de errores
FILE* readInputFile(string);	//abre archivo y comprueba que todo este bien
void fetch();					//lee el sig char
string lexado(FILE*);			//separa en tokens el codigo de entrada
string ident(FILE*);			//identifica palabras clave o identificadores
string numero(FILE*);			//identifica numeros
string readString(FILE*);		//si se lee un ' se toma como inicio de string




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//MAIN
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

	if (argc != 2) {
		cout<<"Usage: pl0c.exe filename.pl0"<<endl;
		exit(1);
	}

    string token;
    string filename = argv[1];
    FILE *archivo;

    archivo = readInputFile(filename);

    fread(&lectura, sizeof(char), 1, archivo);

    do{
        token = lexado(archivo);
        cout<<token<<endl;
    } while(token != FIN_PROGRAMA);

    cout<<"Todo termino bien en la linea: "<<linea<<endl;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCIONES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////LEXER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void error(string msj){
	linea == 0 ? cout<<"Error: "<<msj<<endl : cout<<"Error en linea "<<linea<<": "<<msj<<endl;
}

FILE* readInputFile(string filename){

	//me aseguro que tenga .pl0
	if (filename.find(".pl0") == string::npos){
		error("archivo " + filename + " no termina en '.pl0'");
		exit(1);
	}

	FILE *archivo = fopen(filename.c_str(), "r");

	//no se pudo abrir
	if (archivo == NULL){
		error("No se pudo abrir el archivo: " + filename);
		exit(1);
	}
	//se abrio y en archivo tengo el puntero
	else { linea++; return archivo; }

	return NULL;
}

void fetch(FILE *f){
	fread(&lectura, sizeof(char), 1, f);
}

string lexado(FILE *f){

	if(feof(f)) return FIN_PROGRAMA;

    while(lectura == ' ' || lectura == '\n' || lectura == '\t'){
    	if (lectura == '\n') linea++;
    	fetch(f);
    }

    if(isalpha(lectura)) return ident(f);
    if(isdigit(lectura)) return numero(f);

    string simbolo;

    switch(lectura){
        case '.':	simbolo = PUNTO; fetch(f);	break;
        case '=':	simbolo = IGUAL;			break;
        case '+':	simbolo = SUMA;				break;
        case '-':	simbolo = RESTA;			break;
        case ',':	simbolo = COMA;				break;
        case '*':	simbolo = MULTIPLICACION;	break;
        case '/':	simbolo = DIVISION;			break;
        case '(':	simbolo = PARENTESIS_L;		break;
        case ')':	simbolo = PARENTESIS_R;		break;
        case ';':	simbolo = PUNTO_COMA;		break;
        case '"':							//simbolo = COMILLA_DOBLE;	break;
        case '\'':	return readString(f); 	//simbolo = COMILLA_SIMPLE;	break;
        case '>':
            fetch(f);
            if(lectura == '=') simbolo = MAYOR_IGUAL;
            else simbolo = MAYOR;
            break;
        case '<':
            fetch(f);
            switch(lectura){
                case '=':	simbolo = MENOR_IGUAL;	break;
                case '>':	simbolo = DISTINTO;		break;
                default:	simbolo = MENOR;		break;
            }
            break;
        case ':':
            fetch(f);
            if(lectura == '=') simbolo = ASIGNACION;
            else error("caracter inesperado despues del ':'.");
            break;
        default: error("al leer caracter."); break;
    }

    fetch(f);
    return simbolo;
}

string ident(FILE *f){
    string palabra = "";

    while(isalnum(lectura) || lectura == '_'){
    	string letraString(1, lectura);
        palabra += letraString;
        fetch(f);
    }

    //ASD PASAR A UPPER O LOWER CASE

    if(palabra == CONSTANTE || palabra == VARIABLE || palabra == PROCEDURE || palabra == CALL || 
    	palabra == BEGIN || palabra == END || palabra == IF || palabra == THEN || palabra == WHILE || 
    	palabra == DO || palabra == ODD || palabra == ESCRIBIR || palabra == ESCRIBIR_LN || 
    	palabra == LEER_LN)
            return palabra;
    else return IDENT;
}

string numero(FILE *f){

	string numero = "";

    while(isdigit(lectura)){
    	string numeroString(1, lectura);
        numero += numeroString;
        fetch(f);
    }

    //en numero el numero que se leyo

    return NUMERO;
}

string readString(FILE *f){
	string str = "";

	do{
		//entre con un ' asiq la salteo
		fetch(f);
		if (lectura != '\''){
			string lecturaString(1, lectura);
			str += lecturaString;
		}
	}
	while(lectura != '\'');

	//en str tengo la string q se leyo

	fetch(f);
	return STRING;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////PARSER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

























