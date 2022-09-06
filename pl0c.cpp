/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//LIBRERIAS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <string>
#include <cstdlib>

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

typedef struct tablaSimbolos
{
    string nombre;
    string tipo;
    int valor;
    tablaSimbolos *sig;
} tablaSimbolos;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VARIBALES GLOBALES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int linea = 0;		//para saber cuantas lineas tiene el codigo fuente
static char lectura;		//donde se guardan los caracteres que se leen del archivo
static infoLectura tokens;	//donde se guarda la info de lo que se lee
static tablaSimbolos *base; //entrada de la tabla

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CONSTANTES GLOBALES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define	__PUNTO      		"."
#define	__IDENT      		"ident"
#define	__NUMERO     		"num"
#define	__STRING     		"string"
#define	__CONSTANTE  		"const"
#define	__VARIABLE   		"var"
#define	__PROCEDURE  		"procedure"
#define	__CALL       		"call"
#define	__BEGIN      		"begin"
#define	__END        		"end"
#define	__IF         		"if"
#define	__THEN       		"then"
#define	__WHILE      		"while"
#define	__DO         		"do"
#define	__ODD        		"odd"
#define	__ESCRIBIR   		"write"
#define	__ESCRIBIR_LN		"writeln"
#define	__LEER_LN    		"readln"
#define	__IGUAL      		"="
#define	__DISTINTO   		"<>"
#define	__MAYOR      		">"
#define	__MENOR      		"<"
#define	__MAYOR_IGUAL		">="
#define	__MENOR_IGUAL		"<="
#define	__SUMA       		"+"
#define	__RESTA      		"-"
#define	__MULTIPLICACION	"*"
#define	__DIVISION      	"/"
#define	__PARENTESIS_L  	"("
#define	__PARENTESIS_R  	")"
#define	__ASIGNACION    	":="
#define	__PUNTO_COMA    	";"
#define	__COMA			    ","
#define	__COMILLA_SIMPLE	"'"
#define	__COMILLA_DOBLE	    "\""
#define	__FIN_PROGRAMA  	"EOF"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROTOTIPOS DE FUNCIONES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void error(string,int,string);	    // manejo de errores
void errorLex(string);              // manejo de errores lexicos
void errorSintax(string, string);   //manejo de errores sintacticos
void errorSemant(string,int);       //manejo de errores semanticos
FILE* readInputFile(string);	    // abre archivo y comprueba que todo este bien

//LEXER
void fetch(FILE*);		    // lee el sig char
void lexer(FILE*);		    // separa en tokens el codigo de entrada
void ident(FILE*);		    // identifica palabras clave o identificadores
void numero(FILE*);         // identifica numeros
void cadena(FILE*);		    // si se lee un ' se toma como inicio de string

//PARSER
//R2. Declarar para cada grafo un procedimiento que contenga las sentencias resultantes de aplicarle al
//grafo las reglas R3 a R7.
void parser(FILE*);                 // checkea la sintaxis del codigo
void pedirLex(FILE*);               // le pide al lexer el siguiente token/palabra
void expectativa(string,FILE*);     // checkea si el token es el esperado sintacticamente, error si no
void programa(FILE*);               // procesa el grafo de programa del lenguaje
void bloque(FILE*);                 // procesa el grafo de bloque del lenguaje
void proposicion(FILE*);            // procesa el grafo de proposicion del lenguaje
void condicion(FILE*);              // procesa el grafo de condicion del lenguaje
void expresion(FILE*);              // procesa el grafo de expresion del lenguaje
void termino(FILE*);                // procesa el grafo de termino del lenguaje
void factor(FILE*);                 // procesa el grafo de factor del lenguaje
void cadena(FILE*);                 // procesa la lectura de cadenas


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

    fuente = readInputFile(filename);

    if (fuente == NULL) exit(1);

    fetch(fuente);

    cout<<"linea\t|\tS\t\tCAD"<<endl;
    do{
        lexer(fuente);
        parser(fuente);
    } while(tokens.tokenType != __FIN_PROGRAMA);

    cout<<"Todo termino en la linea: "<<linea<<endl;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FUNCIONES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////LEXER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void error(string msj){
    linea == 0 ? cout<<"Error: "<<msj<<endl : cout<<"Error en linea "<<linea<<": "<<msj<<(msj.find(".") ? " " : ". ");
    cout<<endl<<endl;
    //en cuanto detecta un error se sale
    exit(1);
}

void errorLex(string msj){
    linea == 0 ? cout<<"Error: "<<msj<<endl : cout<<"Error en linea "<<linea<<": "<<msj<<(msj.find(".") ? " " : ". ");
    cout<<"No se reconoce el carcater: "<<lectura<<endl;
    cout<<endl<<endl;
    //en cuanto detecta un error se sale
    exit(1);
}

void errorSintax(string msj, string tok){
    linea == 0 ? cout<<"Error: "<<msj<<endl : cout<<"Error en linea "<<linea<<": "<<msj<<(msj.find(".") ? " " : ". ");
    cout<<"Se tiene el token: '"<<tokens.tokenType<<"' y se esperaba: '"<<tok<<"'";
    cout<<endl<<endl;
    //en cuanto detecta un error se sale
    exit(1);
}

void errorSemant(string msj, int lor){
    linea == 0 ? cout<<"Error: "<<msj<<endl : cout<<"Error en linea "<<linea<<": "<<msj<<(msj.find(".") ? " " : ". ");
    cout<<""<<endl;
    cout<<endl<<endl;
    //en cuanto detecta un error se sale
    exit(1);
}

FILE* readInputFile(string filename){

	//me aseguro que tenga .pl0
	if (filename.find(".pl0") == string::npos && filename.find(".PL0") == string::npos)
        error("archivo " + filename + " no termina en '.pl0'");

	FILE *archivo = fopen(filename.c_str(), "r");

	//no se pudo abrir
	if (archivo == NULL)   error("No se pudo abrir el archivo: " + filename);

	//se abrio y en archivo tengo el puntero
	else { linea++; return archivo; }

	return NULL;
}

void fetch(FILE *f){
	fread(&lectura, sizeof(char), 1, f);

    if (feof(f) && tokens.tokenType != __PUNTO){
        errorSintax("falta el punto ('.') final.", ".");
        exit(1);
    }
}
/*
La tarea del analizador léxico consiste en:
 a. Saltear los separadores (blancos, tabulaciones, comentarios).
 b. Reconocer los símbolos válidos e informar sobre los no válidos.
 c. Llevar la cuenta de los renglones del programa.
 d. Copiar los caracteres de entrada a la salida, generando un listado
 con los renglones numerados.
*/
void lexer(FILE *f){

    if(feof(f)){
        tokens.tokenType = tokens.token = __FIN_PROGRAMA;
        return;
    }

    while(lectura == ' ' || lectura == '\n' || lectura == '\t'){
    	if (lectura == '\n') linea++;
    	fetch(f);
    }

    if(isalpha(lectura)){
        ident(f);
        return;
    }
    if(isdigit(lectura)){
        numero(f);
        return;
    }

    switch(lectura){
        case '.':	tokens.tokenType = tokens.token = __PUNTO; fetch(f);  break;
        case '=':	tokens.tokenType = tokens.token = __IGUAL;            break;
        case '+':	tokens.tokenType = tokens.token = __SUMA;             break;
        case '-':	tokens.tokenType = tokens.token = __RESTA;            break;
        case ',':	tokens.tokenType = tokens.token = __COMA;             break;
        case '*':	tokens.tokenType = tokens.token = __MULTIPLICACION;   break;
        case '/':	tokens.tokenType = tokens.token = __DIVISION;         break;
        case '(':	tokens.tokenType = tokens.token = __PARENTESIS_L;     break;
        case ')':	tokens.tokenType = tokens.token = __PARENTESIS_R;     break;
        case ';':	tokens.tokenType = tokens.token = __PUNTO_COMA;       break;
        case '"':                            //tokens.tokenType = tokens.token = __COMILLA_DOBLE;	break;
        case '\'':	cadena(f);   return; //tokens.tokenType = tokens.token = __COMILLA_SIMPLE;	break;
        case '>':
            fetch(f);
            if(lectura == '=') tokens.tokenType = tokens.token = __MAYOR_IGUAL;
            else tokens.tokenType = tokens.token = __MAYOR;
            break;
        case '<':
            fetch(f);
            switch(lectura){
                case '=':	tokens.tokenType = tokens.token = __MENOR_IGUAL;  break;
                case '>':	tokens.tokenType = tokens.token = __DISTINTO;     break;
                default:	tokens.tokenType = tokens.token = __MENOR;		  break;
            }
            break;
        case ':':
            fetch(f);
            if(lectura == '=') tokens.tokenType = tokens.token = __ASIGNACION;
            else errorLex("caracter inesperado despues del ':'.");
            break;
        default: errorLex("al leer caracter."); break;
    }

    fetch(f);
}

void ident(FILE *f){
    string palabra = "";

    while(isalnum(lectura) || lectura == '_'){
    	string letraString(1, lectura);
        palabra += letraString;
        fetch(f);
    }

    //paso todo a minus
    for (int i = 0; i < palabra.length(); i++) palabra[i] = tolower(palabra[i]);

    if(palabra == __CONSTANTE || palabra == __VARIABLE || palabra == __PROCEDURE || palabra == __CALL ||
    	palabra == __BEGIN || palabra == __END || palabra == __IF || palabra == __THEN || palabra == __WHILE ||
    	palabra == __DO || palabra == __ODD || palabra == __ESCRIBIR || palabra == __ESCRIBIR_LN ||
    	palabra == __LEER_LN)
            tokens.tokenType = tokens.token = palabra;
    else{
        tokens.tokenType = __IDENT;
        tokens.token = palabra;
    }
}

void numero(FILE *f){

	string num = "";

    while(isdigit(lectura)){
    	string numeroString(1, lectura);
        num += numeroString;
        fetch(f);
    }

    tokens.tokenType = __NUMERO;
    tokens.token = num;
}

void cadena(FILE *f){
	string str = "";
    char stringChar = lectura;
	do{
        //entre con un ' o " asiq la salteo
        fetch(f);

        //me encontre el final de string
        if(lectura == stringChar) {
            tokens.tokenType = __STRING;
            tokens.token = str;
            fetch(f);
            return;
        }

        switch(stringChar){
            case '\'':
                //agrego \ para el uso de strings en cpp
                if (lectura == '\"') str += "\\";
            case '\"':
                if(lectura == '\n') linea++;

                string lecturaString(1, lectura);
                str += lecturaString;

                if(feof(f)){
                    tokens.tokenType = tokens.token = __FIN_PROGRAMA;
                    errorLex("string sin terminar.");
                    return;
                }
                break;
        }
	}
	while(lectura != '\'');
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////PARSER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void expectativa(string tokEsperado, FILE* f){

    if (tokEsperado != tokens.tokenType)    {
        errorSintax("error sintactico.", tokEsperado);
    }

    cout<<linea<<"\t|\t"<<tokens.tokenType<<(tokens.tokenType != __PROCEDURE ? "\t\t" : "\t")<<tokens.token<<endl;
    pedirLex(f);
}

void pedirLex(FILE* f){
    lexer(f);
}

void parser(FILE* f){
    //Primer token/palabra para analizar
    programa(f);
}

//programa = <bloque> "."
void programa(FILE* f){
    bloque(f);
    expectativa(__PUNTO, f);
}

//bloque = ["const" <ident> = <numero> ["," <ident> = <numero>] ";"]
//          |["var" <ident> ["," <ident>] ";"]
//          |{"procedure" <ident> ";" <bloque>} <proposicion>
void bloque(FILE* f){

    //["const" <ident> = <numero> ["," <ident> = <numero>] ";"]
    if (tokens.tokenType == __CONSTANTE){
        expectativa(__CONSTANTE, f);
        expectativa(__IDENT, f);
        expectativa(__IGUAL, f);
        expectativa(__NUMERO, f);

        while(tokens.tokenType == __COMA){
            expectativa(__COMA, f);
            expectativa(__IDENT, f);
            expectativa(__IGUAL, f);
            expectativa(__NUMERO, f);
        }

        expectativa(__PUNTO_COMA, f);
    }

    //["var" <ident> ["," <ident>] ";"]
    if (tokens.tokenType == __VARIABLE){
        expectativa(__VARIABLE, f);
        expectativa(__IDENT, f);
        while(tokens.tokenType == __COMA){
            expectativa(__COMA, f);
            expectativa(__IDENT, f);
        }
        expectativa(__PUNTO_COMA, f);
    }

    //{"procedure" <ident> ";" <bloque> ";"}
    while (tokens.tokenType == __PROCEDURE){
        expectativa(__PROCEDURE, f);
        expectativa(__IDENT, f);
        expectativa(__PUNTO_COMA, f);
        bloque(f);
        expectativa(__PUNTO_COMA, f);
    }

    //<proposicion>
    proposicion(f);
}

/*
proposicion =   [<ident> ":=" <expresion>
            | "call" <ident>
            | "begin" <proposicion> { ";" <proposicion> } "end"
            | "if" <condicion> "then" <proposicion>
            | "while" <condicion> "do" <proposicion>
            | "readln" "(" <ident> {"," <ident>} ")"
            | "write" "(" (<expresion> | <cadena>) {"," (<expresion> | <cadena>)} ")"
            | "writeln" ["(" (<expresion> | <cadena>) {"," (<expresion> | <cadena>)} ")"] ]
*/
void proposicion(FILE* f){

        //[<ident> ":=" <expresion>
        if (tokens.tokenType == __IDENT){
            expectativa(__IDENT, f);
            expectativa(__ASIGNACION, f);
            expresion(f);
        }

        //| "call" <ident>
        else if (tokens.tokenType == __CALL){
            expectativa(__CALL, f);
            expectativa(__IDENT, f);
        }

        //| "begin" <proposicion> { ";" <proposicion> } "end"
        else if (tokens.tokenType == __BEGIN){
            expectativa(__BEGIN, f);
            proposicion(f);
            while(tokens.tokenType == __PUNTO_COMA){
                expectativa(__PUNTO_COMA, f);
                proposicion(f);
            }
            expectativa(__END, f);
        }

        //| "if" <condicion> "then" <proposicion>
        else if (tokens.tokenType == __IF){
            expectativa(__IF, f);
            condicion(f);
            expectativa(__THEN, f);
            proposicion(f);
        }

        //| "while" <condicion> "do" <proposicion>
        else if (tokens.tokenType == __WHILE){
            expectativa(__WHILE, f);
            condicion(f);
            expectativa(__DO, f);
            proposicion(f);
        }

        //| "readln" "(" <ident> {"," <ident>} ")"
        else if (tokens.tokenType == __LEER_LN){
            expectativa(__LEER_LN, f);
            expectativa(__PARENTESIS_L, f);
            expectativa(__IDENT, f);
            while(tokens.tokenType == __COMA){
                expectativa(__COMA, f);
                expectativa(__IDENT, f);
            }
            expectativa(__PARENTESIS_R, f);
        }

        //| "write" "(" (<expresion> | <cadena>) {"," (<expresion> | <cadena>)} ")"
        else if (tokens.tokenType == __ESCRIBIR){
            expectativa(__ESCRIBIR, f);
            expectativa(__PARENTESIS_L, f);

            if (tokens.tokenType == __STRING)   expectativa(__STRING, f);
            else    expresion(f);

            while(tokens.tokenType == __COMA){
                expectativa(__COMA, f);
                if (tokens.tokenType == __STRING)   expectativa(__STRING, f);
                else    expresion(f);
            }

            expectativa(__PARENTESIS_R, f);
        }

        //| "writeln" ["(" (<expresion> | <cadena>) {"," (<expresion> | <cadena>)} ")"] ]
        else if (tokens.tokenType == __ESCRIBIR_LN){
            expectativa(__ESCRIBIR_LN, f);

            if (tokens.tokenType == __PARENTESIS_L){
                expectativa(__PARENTESIS_L, f);

                if (tokens.tokenType == __STRING)   expectativa(__STRING, f);
                else    expresion(f);

                while(tokens.tokenType == __COMA){
                    expectativa(__COMA, f);
                    if (tokens.tokenType == __STRING)   expectativa(__STRING, f);
                    else    expresion(f);
                }

                expectativa(__PARENTESIS_R, f);
            }
        }
}

//expresion = ["+" | "-"] <termino> {("+" | "-") <termino>}
void expresion(FILE* f){
    if (tokens.tokenType == __SUMA || tokens.tokenType == __RESTA){
        pedirLex(f);
    }
    termino(f);

    while(tokens.tokenType == __SUMA || tokens.tokenType == __RESTA){
        pedirLex(f);
        termino(f);
    }
}

//termino = <factor> {("*" | "/") <factor>}
void termino(FILE* f){
    factor(f);
    while(tokens.tokenType == __MULTIPLICACION || tokens.tokenType == __DIVISION){
        pedirLex(f);
        factor(f);
    }
}

/*
factor = <ident>
    |<numero>
    |"(" <expresion> ")"
*/
void factor(FILE* f){
    if (tokens.tokenType == __IDENT){
        expectativa(__IDENT, f);
    }
    else if(tokens.tokenType == __NUMERO){
        expectativa(__NUMERO, f);
    }
    else if (tokens.tokenType == __PARENTESIS_L){
        expectativa(__PARENTESIS_L, f);
        expresion(f);
        expectativa(__PARENTESIS_R, f);
    }
}

/*
condicion = "odd" <expresion>
        |<expresion> ("=" | "<>" | "<" | "<=" | ">" | ">=") <expresion>
*/
void condicion(FILE* f){
    if (tokens.tokenType == __ODD){
        expectativa(__ODD, f);
        expresion(f);
    }
    else {
        expresion(f);

        if (tokens.tokenType == __IGUAL || tokens.tokenType == __DISTINTO || tokens.tokenType == __MENOR
             || tokens.tokenType == __MENOR_IGUAL || tokens.tokenType == __MAYOR || tokens.tokenType == __MAYOR_IGUAL){
            pedirLex(f);
        }
        else{
            errorSintax("comparador invalido.", "\"=\", \"<>\", \"<\", \"<=\", \">\" o \">=\".");
        }

        expresion(f);
    }
}














