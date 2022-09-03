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
typedef struct infoLectura
{
    string tokenType;
    string token;
} infoLectura;

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
void error(string);                     //manejo de errores
FILE* readInputFile(string);            //abre archivo y comprueba que todo este bien

//LEXER
void fetch();                           //lee el sig char
void lexer(FILE*,infoLectura*);         //separa en tokens el codigo de entrada
void ident(FILE*,infoLectura*);         //identifica palabras clave o identificadores
void numero(FILE*,infoLectura*);        //identifica numeros
void readString(FILE*,infoLectura*);    //si se lee un ' se toma como inicio de string

//PARSER





/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//MAIN
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

	if (argc != 2) {
		cout<<"Usage: pl0c.exe filename.pl0"<<endl;
		exit(1);
	}

/*
    procedure scanner (var Fuente, Listado: archivo; var S: terminal; var Cad: str63; var Restante: string;
                        var NumLinea: integer);
*/
    //var Fuente: archivo
    FILE *fuente;
    string filename = argv[1];
    
    //var Listado: archivo
    FILE *salida;
    
    //var S: terminal;
    //En el struct linea 18
    
    //var Cad: str63;
    //En el struct linea 18
    
    //var NumLinea: integer
    //variable global en la linea 23
    
    infoLectura tokens;

    fuente = readInputFile(filename);

    fread(&lectura, sizeof(char), 1, fuente);

    cout<<"linea\t|\tS\t\tCAD"<<endl;
    do{
        lexer(fuente, &tokens);
        cout<<linea<<"\t|\t"<<tokens.tokenType<<(tokens.tokenType != PROCEDURE? "\t\t" : "\t")<<tokens.token<<endl;
    } while(tokens.tokenType != FIN_PROGRAMA);

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

void lexer(FILE *f, infoLectura *il){

	if(feof(f)){
        il->tokenType = il->token = FIN_PROGRAMA;
        return;
    }

    while(lectura == ' ' || lectura == '\n' || lectura == '\t'){
    	if (lectura == '\n') linea++;
    	fetch(f);
    }

    if(isalpha(lectura)){
        ident(f, il);
        return;
    }
    if(isdigit(lectura)){
        numero(f, il);
        return;
    }

    switch(lectura){
        case '.':	il->tokenType = il->token = PUNTO; fetch(f);  break;
        case '=':	il->tokenType = il->token = IGUAL;            break;
        case '+':	il->tokenType = il->token = SUMA;             break;
        case '-':	il->tokenType = il->token = RESTA;            break;
        case ',':	il->tokenType = il->token = COMA;             break;
        case '*':	il->tokenType = il->token = MULTIPLICACION;   break;
        case '/':	il->tokenType = il->token = DIVISION;         break;
        case '(':	il->tokenType = il->token = PARENTESIS_L;     break;
        case ')':	il->tokenType = il->token = PARENTESIS_R;     break;
        case ';':	il->tokenType = il->token = PUNTO_COMA;       break;
        case '"':                                //il->tokenType = il->token = COMILLA_DOBLE;	break;
        case '\'':	readString(f, il);   return; //il->tokenType = il->token = COMILLA_SIMPLE;	break;
        case '>':
            fetch(f);
            if(lectura == '=') il->tokenType = il->token = MAYOR_IGUAL;
            else il->tokenType = il->token = MAYOR;
            break;
        case '<':
            fetch(f);
            switch(lectura){
                case '=':	il->tokenType = il->token = MENOR_IGUAL;  break;
                case '>':	il->tokenType = il->token = DISTINTO;     break;
                default:	il->tokenType = il->token = MENOR;		  break;
            }
            break;
        case ':':
            fetch(f);
            if(lectura == '=') il->tokenType = il->token = ASIGNACION;
            else error("caracter inesperado despues del ':'.");
            break;
        default: error("al leer caracter."); break;
    }

    fetch(f);
}

void ident(FILE *f, infoLectura *il){
    string palabra = "";

    while(isalnum(lectura) || lectura == '_'){
    	string letraString(1, lectura);
        palabra += letraString;
        fetch(f);
    }

    //paso todo a minus
    for (int i = 0; i < palabra.length(); i++) palabra[i] = tolower(palabra[i]);

    if(palabra == CONSTANTE || palabra == VARIABLE || palabra == PROCEDURE || palabra == CALL || 
    	palabra == BEGIN || palabra == END || palabra == IF || palabra == THEN || palabra == WHILE || 
    	palabra == DO || palabra == ODD || palabra == ESCRIBIR || palabra == ESCRIBIR_LN || 
    	palabra == LEER_LN)
            il->tokenType = il->token = palabra;
    else{
        il->tokenType = IDENT;
        il->token = palabra;
    }
}

void numero(FILE *f, infoLectura *il){

	string num = "";

    while(isdigit(lectura)){
    	string numeroString(1, lectura);
        num += numeroString;
        fetch(f);
    }

    il->tokenType = NUMERO;
    il->token = num;
}

void readString(FILE *f, infoLectura *il){
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
    il->tokenType = STRING;
    il->token = str;

	fetch(f);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////PARSER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
