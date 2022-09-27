/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//LIBRERIAS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//USING NAMESPACE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace std;

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
#define __END_O_PC         "end' o ';"
#define __COMA_O_PC         ",' o ';"
#define __PAREN_O_C         ")' o ',"

//PARA VERIFICACIONES SEMANTICAS Y TABLA DE SIMBOLOS
#define LEFT    500
#define RIGHT   501
#define CALL    502
#define MAX     256

//DIRECCIONES DE MEMORIA DONDE ESTAN RUTINAS E/S
#define IO_MOSTRAR_STRING       0x03E0  // muestra por consola una cadena terminada en 0, alojada en la dir guardada en EAX.
#define IO_SALTO_LINEA_CONSOLA  0x0410  // envía un salto de línea a la consola.
#define IO_MOSTRAR_EAX_CONSOLA  0x0420  // muestra por consola el número entero contenido en EAX.
#define IO_FINALIZA_PROGRAMA    0x0588  // finaliza el programa
#define IO_LECTURA_TECLADO      0x0590  // lee por consola un número entero y lo deja guardado en EAX.
#define __LONG_MAX              4294967296
#define ENCABEZADO              0x200
#define LARGO_BYTES_JMP_CALL    0x05
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//STRUCTS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct infoLectura
{
    string tokenType;
    string token;
} infoLectura;

struct tablaSimbolos
{
    string nombre;
    string tipo;
    int valor;
};
typedef tablaSimbolos arrSimbolos [MAX];
typedef char arr4Bytes[4];

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VARIBALES GLOBALES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int linea = 0;		//para saber cuantas lineas tiene el codigo fuente
static char lectura;		//donde se guardan los caracteres que se leen del archivo
static infoLectura tokens;	//donde se guarda la info de lo que se lee
static arrSimbolos simbTab; //tabla de simbolos
static unsigned char memoria[8192]; //donde se guardan los bytes de las instrucciones x86
static int tamMemoria;      //donde se guarda el tamaño final de la memoria

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PROTOTIPOS DE FUNCIONES
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void error(string);         	    // manejo de errores
void errorLex(string);              // manejo de errores lexicos
void errorSintax(string, string);   //manejo de errores sintacticos
void errorSemant(string,string);    //manejo de errores semanticos
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
void parser(FILE*);                     // checkea la sintaxis del codigo
void pedirLex(FILE*);                   // le pide al lexer el siguiente token/palabra
void expectativa(string,FILE*);         // checkea si el token es el esperado sintacticamente, error si no
void programa(FILE*);                   // procesa el grafo de programa del lenguaje
void bloque(FILE*,int&,int&,int,int&);  // procesa el grafo de bloque del lenguaje
void proposicion(FILE*,int&,int,int);   // procesa el grafo de proposicion del lenguaje
void condicion(FILE*,int&,int,int);     // procesa el grafo de condicion del lenguaje
void expresion(FILE*,int&,int,int);     // procesa el grafo de expresion del lenguaje
void termino(FILE*,int&,int,int);       // procesa el grafo de termino del lenguaje
void factor(FILE*,int&,int,int);        // procesa el grafo de factor del lenguaje

//SEMANTICA (tabla de simbolos)
void agregarSimbolo(string,int,int,int&);   // agrega el simbolo recivido a la tabla
void verificarIdentificador(int,int,int);   // verifica, al hacer una asignacion, parte izquierda (deberia ser ident)
                                            // parte derecha (no puede ser un procedure)
                                            // y si al hacer un call se uso un procedure (tiene que)
void incDesplazamiento(int&,int);           // incrementa desplazamiento, error si se pasa del max

//GENERADOR DE CODIGO
void cgInit();                          // inicializa las primeras pos. de memoria que siempre van a ser igual
void inttchar(long long int,arr4Bytes);           // pone en un array de chars los 4 bytes del valor del int (en little endian)
void mem0(int&,int);                    // pone en memoria x cantidad de 0s
void opMoveEDI(int&);                   // pone el opcode para la operacion MOVE EDI en memoria
void opMoveEAX_EDI(int&,int);           // pone el opcode para la operacion MOVE EAX, EDI en memoria
void opMoveEDI_EAX(int&,int);           // pone el opcode para la operacion MOVE EDI, EAX en memoria
void opMoveEAX(int&,int);               // pone el opcode para la operacion MOVE EAX en memoria
void opXCHG(int&);                      // pone el opcode para la operacion XCHG en memoria
void opPushEAX(int&);                   // pone el opcode para la operacion PUSH EAX en memoria
void opPopEAX(int&);                    // pone el opcode para la operacion POP EAX en memoria
void opPopEBX(int&);                    // pone el opcode para la operacion POP EBX en memoria
void opADD(int&);                       // pone el opcode para la operacion ADD en memoria
void opSUBB(int&);                      // pone el opcode para la operacion SUBB en memoria
void opIMUL(int&);                      // pone el opcode para la operacion IMUL en memoria
void opIDIV(int&);                      // pone el opcode para la operacion IDIV en memoria
void opCDQ(int&);                       // pone el opcode para la operacion CDQ en memoria
void opNEG(int&);                       // pone el opcode para la operacion NEG en memoria
void opTEST(int&,char);                 // pone el opcode para la operacion TEST en memoria
void opCMP(int&);                       // pone el opcode para la operacion CMP en memoria
void opJE(int&);                        // pone el opcode para la operacion JE en memoria
void opJNE(int&);                       // pone el opcode para la operacion JNE en memoria
void opJL(int&);                        // pone el opcode para la operacion JL en memoria
void opJLE(int&);                       // pone el opcode para la operacion JLE en memoria
void opJG(int&);                        // pone el opcode para la operacion JG en memoria
void opJGE(int&);                       // pone el opcode para la operacion JGE en memoria
void opJPO(int&);                       // pone el opcode para la operacion JPO en memoria
void opJMP(int&,int);                   // pone el opcode para la operacion JMP en memoria
void opCALL(int&,int);                  // pone el opcode para la operacion CALL en memoria
void opRET(int&);                       // pone el opcode para la operacion RET en memoria
int getSymbolValue(string,int,int);     // devuelve el valor del identificador
string getSymbolType(string,int,int);   // devuelve el tipo del identificador
void arreglarMem4bytes(int,int);        // arregla la posicion de memoria con el valor que se le pase
int byte4toint(char,char,char,char);    // devuelve un int a partir de 4 bytes (del mas signif. al menos)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//MAIN
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){

	if (argc != 2) {
		cout<<"Usage: pl0c.exe filename.pl0"<<endl;
		exit(1);
	}

    FILE *fuente;
    string filename = argv[1];

    fuente = readInputFile(filename);
    if (fuente == NULL) exit(1);

    fetch(fuente);

    cout<<"linea\t|\tS\t\tCAD"<<endl;
    do{
        lexer(fuente);
        parser(fuente);
    } while(tokens.tokenType != __FIN_PROGRAMA);

    cout<<"Todo termino en la linea: "<<linea<<endl;

    cout<<"\n\n\n\tTABLA DE SIMBOLOS AL FINAL DEL PROGRAMA:\n"<<endl;
    for(int i = 0; i <= 15; i++)
        cout<<"\t\tNOMBRE:"<<simbTab[i].nombre<<"\t->\tTIPO:"<<simbTab[i].tipo<<"\t->\tVALOR:"<<simbTab[i].valor<<endl;

    cout<<"\n\n\n\tARRAY MEMORIA:\n"<<endl;
    cout<<"Offset\t0\t1\t2\t3\t4\t5\t6\t7\t\t8\t9\tA\t\tB\tC\tD\tE\tF"<<endl;

    for(int i = 1792; i <= 2000; i+=16){
        cout<<hex<<uppercase<<i<<"\t";
        for (int j = i; j < i+16; j++){
            if(memoria[j] < 16) cout<<"0"<<(int)memoria[j]<<"\t";
            else cout<<(int)memoria[j]<<"\t";

            if(j == i+7) cout<<"\t";
        }
        cout<<endl;
    }

    FILE* exe;
    string exeFile = filename;

    //elimino rutas en el nombre del archivo
    int ind = exeFile.find_last_of('/');
    if (ind != string::npos) exeFile = exeFile.substr(ind+1, exeFile.length()-ind);
    exeFile.replace((exeFile.length()) - 3, 3, "exe");

    exe = fopen(exeFile.c_str(), "wb+");

    for(int i = 0; i < tamMemoria; i++)
        fwrite(&memoria[i], sizeof(char), 1, exe);

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
    cout<<"Se tiene el token: '"<<tokens.token<<"' y se esperaba: '"<<tok<<"'";
    cout<<endl<<endl;
    //en cuanto detecta un error se sale
    exit(1);
}

void errorSemant(string msj, string token){
    linea == 0 ? cout<<"Error: "<<msj<<endl : cout<<"Error en linea "<<linea<<": "<<msj<<token<<"'";
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

void fetchSTR(FILE *f){
	fread(&lectura, sizeof(char), 1, f);
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
        case '\'':	cadena(f);   return;
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
        default: errorLex("al leer caracter desconocido."); break;
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

    string palabraNormal = palabra;

    //paso todo a minus
    for (int i = 0; i < palabra.length(); i++) palabra[i] = tolower(palabra[i]);

    if(palabra == __CONSTANTE || palabra == __VARIABLE || palabra == __PROCEDURE || palabra == __CALL ||
        palabra == __BEGIN || palabra == __END || palabra == __IF || palabra == __THEN || palabra == __WHILE ||
        palabra == __DO || palabra == __ODD || palabra == __ESCRIBIR || palabra == __ESCRIBIR_LN ||
        palabra == __LEER_LN){
            tokens.tokenType = palabra;
            tokens.token = palabraNormal;
    }
    else{
        tokens.tokenType = __IDENT;
        tokens.token = palabraNormal;
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

	do{
        //entre con un ' o " asiq la salteo
        fetchSTR(f);

        //me encontre el final de string
        if(lectura == '\'') {
            tokens.tokenType = __STRING;
            tokens.token = str;
            fetchSTR(f);
            return;
        }

        if(lectura == '\n') linea++;
        string lecturaString(1, lectura);
        str += lecturaString;

        if(feof(f)){
            tokens.tokenType = tokens.token = __FIN_PROGRAMA;
            error("string sin terminar.");
        }
	}
	while(lectura != '\'');
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////PARSER
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void expectativa(string tokEsperado, FILE* f){

    if (tokEsperado != tokens.tokenType){
        errorSintax("error sintactico.", tokEsperado);
    }

    cout<<linea<<"\t|\t"<<tokens.tokenType<<(tokens.tokenType != __PROCEDURE ? "\t\t" : "\t")<<tokens.token<<endl;
    pedirLex(f);
}

void pedirLex(FILE* f){
    lexer(f);
}

void parser(FILE* f){
    cgInit();
    programa(f);
}

void incDesplazamiento(int &des, int base){
    des++;
    if((base+des) >= MAX) error("se llego a la maxima cantidad de identificacores posibles.");
}

//programa = <bloque> "."
void programa(FILE* f){
    int varDir = 0;
    int indiceMemoria = 1797;
    int cantVar = 0;

    bloque(f, indiceMemoria, varDir, 0, cantVar);
    expectativa(__PUNTO, f);

    //GENERACION DE CODIGO//=======================================================================
    opJMP(indiceMemoria, IO_FINALIZA_PROGRAMA - indiceMemoria); // salto a rutina de e/s que finaliza el programa

    int BaseOfCode = byte4toint(memoria[207], memoria[206], memoria[205], memoria[204]);
    int ImageBase = byte4toint(memoria[215], memoria[214], memoria[213], memoria[212]);

    // arreglo direccion de la tabla de simbolos, resto el encabezado (200) y la propia instruccion (5)
    arreglarMem4bytes(1793, BaseOfCode + ImageBase + indiceMemoria - ENCABEZADO);

    for (int i = 0; i < cantVar; i++)                           // pongo 4 0's por variable en el programa
        mem0(indiceMemoria, 4);

    arreglarMem4bytes(416, indiceMemoria - ENCABEZADO);         // arreglo VirtualSize con el tamaño actual de text section

    int fileAlignment = byte4toint(memoria[223], memoria[222], memoria[221], memoria[220]);
    while((indiceMemoria % fileAlignment) != 0)
        mem0(indiceMemoria, 1);                                 // inserto 0's para q tamaño sea multiplo de FileAlignment

    tamMemoria = indiceMemoria;

    arreglarMem4bytes(188, indiceMemoria - ENCABEZADO);         // ajusto SizeOfCodeSection con el tamaño de text
    arreglarMem4bytes(424, indiceMemoria - ENCABEZADO);         // ajusto SizeOfRawData con el tamaño de text

    int SizeOfCodeSection = byte4toint(memoria[191], memoria[190], memoria[189], memoria[188]);
    int SizeOfRawData = byte4toint(memoria[427], memoria[426], memoria[425], memoria[424]);
    int SectionAlignment = byte4toint(memoria[219], memoria[218], memoria[217], memoria[216]);
    //SizeOfImage (240) =
    //  (2 + SizeOfCodeSection / SectionAlignment) * SectionAlignment
    arreglarMem4bytes(240, ((2 + SizeOfCodeSection / SectionAlignment) * SectionAlignment));

    //BaseOfData (208) =
    //  (2 + SizeOfRawData / SectionAlignment) * SectionAlignment
    arreglarMem4bytes(208, ((2 + SizeOfRawData / SectionAlignment) * SectionAlignment));
    //=============================================================================================
}

//bloque = ["const" <ident> = <numero> ["," <ident> = <numero>] ";"]
//          |["var" <ident> ["," <ident>] ";"]
//          |{"procedure" <ident> ";" <bloque>} <proposicion>
void bloque(FILE* f, int &indMem, int &varDir, int base, int &cantVar){
    int desplazamiento = 0;
    int indMemProcedureArreglar = 0;

    opJMP(indMem, 0x00);
    indMemProcedureArreglar = indMem;

    //["const" <ident> = <numero> ["," <ident> = <numero>] ";"]
    if (tokens.tokenType == __CONSTANTE){
        expectativa(__CONSTANTE, f);

        if(tokens.tokenType == __IDENT) agregarSimbolo(__CONSTANTE, base, desplazamiento, varDir);
        expectativa(__IDENT, f);
        incDesplazamiento(desplazamiento,base);

        expectativa(__IGUAL, f);

        if(tokens.tokenType == __NUMERO) agregarSimbolo(__NUMERO, base, desplazamiento, varDir);
        expectativa(__NUMERO, f);

        while(tokens.tokenType == __COMA){
            expectativa(__COMA, f);

            if(tokens.tokenType == __IDENT) agregarSimbolo(__CONSTANTE, base, desplazamiento, varDir);
            expectativa(__IDENT, f);
            incDesplazamiento(desplazamiento,base);

            expectativa(__IGUAL, f);

            if(tokens.tokenType == __NUMERO) agregarSimbolo(__NUMERO, base, desplazamiento, varDir);
            expectativa(__NUMERO, f);
        }

        if(tokens.tokenType != __PUNTO_COMA) expectativa(__COMA_O_PC, f);
        else expectativa(__PUNTO_COMA, f);
    }

    //["var" <ident> ["," <ident>] ";"]
    if (tokens.tokenType == __VARIABLE){
        expectativa(__VARIABLE, f);

        if(tokens.tokenType == __IDENT){
            agregarSimbolo(__VARIABLE, base, desplazamiento, varDir);
            cantVar++;
            varDir += 4;
            incDesplazamiento(desplazamiento,base);
        }
        expectativa(__IDENT, f);

        while(tokens.tokenType == __COMA){
            expectativa(__COMA, f);

            if(tokens.tokenType == __IDENT){
                agregarSimbolo(__VARIABLE, base, desplazamiento, varDir);
                cantVar++;
                varDir += 4;
                incDesplazamiento(desplazamiento,base);
            }
            expectativa(__IDENT, f);
        }
        if(tokens.tokenType != __PUNTO_COMA) expectativa(__COMA_O_PC, f);
        else expectativa(__PUNTO_COMA, f);
    }

    //{"procedure" <ident> ";" <bloque> ";"}
    while (tokens.tokenType == __PROCEDURE){
        expectativa(__PROCEDURE, f);

        if(tokens.tokenType == __IDENT) agregarSimbolo(__PROCEDURE, base, desplazamiento, indMem);
        expectativa(__IDENT, f);
        incDesplazamiento(desplazamiento,base);

        expectativa(__PUNTO_COMA, f);
        bloque(f, indMem, varDir, base+desplazamiento, cantVar);
        expectativa(__PUNTO_COMA, f);

        //GENERACION DE CODIGO
        opRET(indMem);
        //====================
    }

    //GENERACION DE CODIGO//=============================================================
    arreglarMem4bytes(indMemProcedureArreglar - 4, indMem - indMemProcedureArreglar);
    //===================================================================================

    //<proposicion>
    proposicion(f, indMem, base, desplazamiento);
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
void proposicion(FILE* f, int &indMem, int base, int desplazamiento){
    int identValor = 0;
    int indMemArreglar = 0;
    int indMemJMP = 0;

    //[<ident> ":=" <expresion>
    if (tokens.tokenType == __IDENT){
        if((identValor = getSymbolValue(tokens.token, base, desplazamiento)) == -1)
            error("al buscar valor de variable.");
        verificarIdentificador(LEFT, base, desplazamiento);
        expectativa(__IDENT, f);
        expectativa(__ASIGNACION, f);
        expresion(f, indMem, base, desplazamiento);

        //GENERACION DE CODIGO//=============
        opPopEAX(indMem);
        opMoveEDI_EAX(indMem, identValor);
        //===================================
    }

    //| "call" <ident>
    else if (tokens.tokenType == __CALL){
        expectativa(__CALL, f);

        if (tokens.tokenType == __IDENT){
            verificarIdentificador(CALL, base, desplazamiento);
            if((identValor = getSymbolValue(tokens.token, base, desplazamiento)) == -1)
                error("al buscar valor de procedure.");
        }

        expectativa(__IDENT, f);

        //GENERACION DE CODIGO//==================
        opCALL(indMem, identValor - indMem);    // 5 bytes de la propia instruccion
        //========================================
    }

    //| "begin" <proposicion> { ";" <proposicion> } "end"
    else if (tokens.tokenType == __BEGIN){
        expectativa(__BEGIN, f);
        proposicion(f, indMem, base, desplazamiento);
        while(tokens.tokenType == __PUNTO_COMA){
            expectativa(__PUNTO_COMA, f);
            proposicion(f, indMem, base, desplazamiento);
        }

        if (tokens.tokenType != __END)  expectativa(__END_O_PC, f);
        else expectativa(__END, f);
    }

    //| "if" <condicion> "then" <proposicion>
    else if (tokens.tokenType == __IF){
        expectativa(__IF, f);
        condicion(f, indMem, base, desplazamiento);

        //guardo la memoria para arreglar el JMP que generó condicion
        indMemArreglar = indMem;

        expectativa(__THEN, f);
        proposicion(f, indMem, base, desplazamiento);

        //GENERACION DE CODIGO//==========================================
        arreglarMem4bytes(indMemArreglar - 4, indMem - indMemArreglar);     //4 bytes del salto (sin incluir la instrucc. misma)
        //================================================================
    }

    //| "while" <condicion> "do" <proposicion>
    else if (tokens.tokenType == __WHILE){
        expectativa(__WHILE, f);

        indMemJMP = indMem;
        condicion(f, indMem, base, desplazamiento);

        indMemArreglar = indMem;
        expectativa(__DO, f);
        proposicion(f, indMem, base, desplazamiento);

        //GENERACION DE CODIGO//===========================================
        arreglarMem4bytes(indMemArreglar - 4, indMem - indMemArreglar + 5);
        opJMP(indMem, indMemJMP - indMem);
        //=================================================================
    }

    //| "readln" "(" <ident> {"," <ident>} ")"
    else if (tokens.tokenType == __LEER_LN){
        expectativa(__LEER_LN, f);
        expectativa(__PARENTESIS_L, f);

        if(tokens.tokenType == __IDENT) {
            verificarIdentificador(LEFT, base, desplazamiento);
            identValor = getSymbolValue(tokens.token,base,desplazamiento);

            //GENERACION DE CODIGO//========================
            opCALL(indMem, IO_LECTURA_TECLADO - indMem);
            opMoveEDI_EAX(indMem, identValor);
            //==============================================
        }

        expectativa(__IDENT, f);

        while(tokens.tokenType == __COMA){
            expectativa(__COMA, f);

            if(tokens.tokenType == __IDENT) {
                verificarIdentificador(LEFT, base, desplazamiento);
                identValor = getSymbolValue(tokens.token,base,desplazamiento);

                //GENERACION DE CODIGO//========================
                opCALL(indMem, IO_LECTURA_TECLADO - indMem);
                opMoveEDI_EAX(indMem, identValor);
                //==============================================
            }

            expectativa(__IDENT, f);
        }

        if(tokens.tokenType != __PARENTESIS_R) expectativa(__PAREN_O_C, f);
        else expectativa(__PARENTESIS_R, f);
    }

    //| "write" "(" (<expresion> | <cadena>) {"," (<expresion> | <cadena>)} ")"
    else if (tokens.tokenType == __ESCRIBIR){
        expectativa(__ESCRIBIR, f);
        expectativa(__PARENTESIS_L, f);

        if (tokens.tokenType == __STRING){
            //GENERACION DE CODIGO//===============================================================
            int BaseOfCode = byte4toint(memoria[207], memoria[206], memoria[205], memoria[204]);    //ambos son para
            int ImageBase = byte4toint(memoria[215], memoria[214], memoria[213], memoria[212]);     //calc. pos. absoluta

            opMoveEAX(indMem, (BaseOfCode + ImageBase + indMem - ENCABEZADO + 0x0A + LARGO_BYTES_JMP_CALL)); //necesito pos. abs.
            opCALL(indMem, IO_MOSTRAR_STRING - indMem);
            opJMP(indMem, tokens.token.length()+1);                                                 //salto la cadena
                                                                                                    //a continuacion
            indMemArreglar = indMem;

            for (int i = 0; i < tokens.token.length(); i++)
                memoria[indMem++] = (unsigned char)tokens.token[i];
            mem0(indMem,1);
            //=====================================================================================

            expectativa(__STRING, f);
        }
        else{
            expresion(f, indMem, base, desplazamiento);

            //GENERACION DE CODIGO//========================
            opPopEAX(indMem);
            opCALL(indMem, IO_MOSTRAR_EAX_CONSOLA - indMem);
            //==============================================
        }

        while(tokens.tokenType == __COMA){
            expectativa(__COMA, f);
            if (tokens.tokenType == __STRING){
                //GENERACION DE CODIGO//===============================================================
                int BaseOfCode = byte4toint(memoria[207], memoria[206], memoria[205], memoria[204]);
                int ImageBase = byte4toint(memoria[215], memoria[214], memoria[213], memoria[212]);

                opMoveEAX(indMem, (BaseOfCode + ImageBase + indMem - ENCABEZADO + (LARGO_BYTES_JMP_CALL * 3)));
                opCALL(indMem, IO_MOSTRAR_STRING - indMem);
                opJMP(indMem, tokens.token.length()+1);

                indMemArreglar = indMem;

                for (int i = 0; i < tokens.token.length(); i++)
                    memoria[indMem++] = (unsigned char)tokens.token[i];
                mem0(indMem,1);
                //=====================================================================================

                expectativa(__STRING, f);
            }
            else{
                expresion(f, indMem, base, desplazamiento);

                //GENERACION DE CODIGO//========================
                opPopEAX(indMem);
                opCALL(indMem, IO_MOSTRAR_EAX_CONSOLA - indMem);
                //==============================================
            }
        }

        if(tokens.tokenType != __PARENTESIS_R) expectativa(__PAREN_O_C, f);
        else expectativa(__PARENTESIS_R, f);
    }

    //| "writeln" ["(" (<expresion> | <cadena>) {"," (<expresion> | <cadena>)} ")"] ]
    else if (tokens.tokenType == __ESCRIBIR_LN){
        expectativa(__ESCRIBIR_LN, f);

        if (tokens.tokenType == __PARENTESIS_L){
            expectativa(__PARENTESIS_L, f);

            if (tokens.tokenType == __STRING){
                //GENERACION DE CODIGO//===============================================================
                int BaseOfCode = byte4toint(memoria[207], memoria[206], memoria[205], memoria[204]);
                int ImageBase = byte4toint(memoria[215], memoria[214], memoria[213], memoria[212]);

                opMoveEAX(indMem, (BaseOfCode + ImageBase + indMem - ENCABEZADO + (LARGO_BYTES_JMP_CALL*3)));
                opCALL(indMem, IO_MOSTRAR_STRING - indMem);
                opJMP(indMem, tokens.token.length()+1);

                indMemArreglar = indMem;

                for (int i = 0; i < tokens.token.length(); i++)
                    memoria[indMem++] = (unsigned char)tokens.token[i];
                mem0(indMem,1);
                //=====================================================================================

                expectativa(__STRING, f);
            }
            else{
                expresion(f, indMem, base, desplazamiento);

                //GENERACION DE CODIGO//========================
                opPopEAX(indMem);
                opCALL(indMem, IO_MOSTRAR_EAX_CONSOLA - indMem);
                //==============================================
            }

            while(tokens.tokenType == __COMA){
                expectativa(__COMA, f);

                if (tokens.tokenType == __STRING){
                    //GENERACION DE CODIGO//===============================================================
                    int BaseOfCode = byte4toint(memoria[207], memoria[206], memoria[205], memoria[204]);
                    int ImageBase = byte4toint(memoria[215], memoria[214], memoria[213], memoria[212]);

                    opMoveEAX(indMem, (BaseOfCode + ImageBase + indMem - ENCABEZADO + (LARGO_BYTES_JMP_CALL*3)));
                    opCALL(indMem, IO_MOSTRAR_STRING - indMem);
                    opJMP(indMem, tokens.token.length()+1);

                    indMemArreglar = indMem;

                    for (int i = 0; i < tokens.token.length(); i++)
                        memoria[indMem++] = (unsigned char)tokens.token[i];
                    mem0(indMem,1);
                    //=====================================================================================

                    expectativa(__STRING, f);
                }
                else{
                    expresion(f, indMem, base, desplazamiento);

                    //GENERACION DE CODIGO//=========================
                    opPopEAX(indMem);
                    opCALL(indMem, IO_MOSTRAR_EAX_CONSOLA - indMem);
                    //===============================================
                }
            }

            if(tokens.tokenType != __PARENTESIS_R) expectativa(__PAREN_O_C, f);
            else expectativa(__PARENTESIS_R, f);
        }
        opCALL(indMem, IO_SALTO_LINEA_CONSOLA - indMem);
    }
}

/*
condicion = "odd" <expresion>
        |<expresion> ("=" | "<>" | "<" | "<=" | ">" | ">=") <expresion>
*/
void condicion(FILE* f, int &indMem, int base, int desplazamiento){
    if (tokens.tokenType == __ODD){
        expectativa(__ODD, f);
        expresion(f, indMem, base, desplazamiento);

        //GENERACION DE CODIGO//===
        opPopEAX(indMem);
        opTEST(indMem, 0x01);           // x & 1
        opJPO(indMem);
        opJMP(indMem, 0x00);            //dsps se arregla, no se sabe la direccion en este momento
        //=========================
    }
    else {
        string comparador;

        expresion(f, indMem, base, desplazamiento);

        if (tokens.tokenType == __IGUAL || tokens.tokenType == __DISTINTO || tokens.tokenType == __MENOR
             || tokens.tokenType == __MENOR_IGUAL || tokens.tokenType == __MAYOR || tokens.tokenType == __MAYOR_IGUAL){
            comparador = tokens.tokenType;
            pedirLex(f);
        }
        else    errorSintax("comparador invalido.", "\"=\", \"<>\", \"<\", \"<=\", \">\" o \">=\".");

        expresion(f, indMem, base, desplazamiento);

        //GENERACION DE CODIGO//==========================================
        opPopEAX(indMem);
        opPopEBX(indMem);
        opCMP(indMem);

        if (comparador == __IGUAL)              opJE(indMem);
        else if (comparador == __DISTINTO)      opJNE(indMem);
        else if (comparador == __MENOR)         opJL(indMem);
        else if (comparador == __MENOR_IGUAL)   opJLE(indMem);
        else if (comparador == __MAYOR)         opJG(indMem);
        else if (comparador == __MAYOR_IGUAL)   opJGE(indMem);

        opJMP(indMem,0x00);
        //================================================================
    }
}

//expresion = ["+" | "-"] <termino> {("+" | "-") <termino>}
void expresion(FILE* f, int &indMem, int base, int desplazamiento){
    string operador;

    if (tokens.tokenType == __SUMA || tokens.tokenType == __RESTA){
        operador = tokens.tokenType;
        pedirLex(f);
    }

    termino(f, indMem, base, desplazamiento);

    //GENERACION DE CODIGO//=======
    if(operador == __RESTA){
        opPopEAX(indMem);
        opNEG(indMem);
        opPushEAX(indMem);
    }
    //=============================

    while(tokens.tokenType == __SUMA || tokens.tokenType == __RESTA){
        operador = tokens.tokenType;

        pedirLex(f);
        termino(f, indMem, base, desplazamiento);

        //GENERACION DE CODIGO//===========================
        if (operador == __SUMA){
            opPopEAX(indMem);
            opPopEBX(indMem);
            opADD(indMem);
            opPushEAX(indMem);
        }
        else if (operador == __RESTA){
            opPopEAX(indMem);
            opPopEBX(indMem);
            opXCHG(indMem);
            opSUBB(indMem);
            opPushEAX(indMem);
        }
        //=================================================
    }
}

//termino = <factor> {("*" | "/") <factor>}
void termino(FILE* f, int &indMem, int base, int desplazamiento){
    factor(f, indMem, base, desplazamiento);
    while(tokens.tokenType == __MULTIPLICACION || tokens.tokenType == __DIVISION){
        string operador = tokens.tokenType;

        pedirLex(f);
        factor(f, indMem, base, desplazamiento);

        //GENERACION DE CODIGO//===========================
        if (operador == __MULTIPLICACION){
            opPopEAX(indMem);
            opPopEBX(indMem);
            opIMUL(indMem);
            opPushEAX(indMem);
        }
        else if (operador == __DIVISION){
            opPopEAX(indMem);
            opPopEBX(indMem);
            opXCHG(indMem);
            opCDQ(indMem);
            opIDIV(indMem);
            opPushEAX(indMem);
        }
        else error("inesperado al operar factores.");
        //=================================================
    }
}

/*
factor = <ident>
    |<numero>
    |"(" <expresion> ")"
*/
void factor(FILE* f, int &indMem, int base, int desplazamiento){
    if (tokens.tokenType == __IDENT){
        verificarIdentificador(RIGHT, base, desplazamiento);

        //GENERACION DE CODIGO//====================================================
        string tipo = getSymbolType(tokens.token, base, desplazamiento);
        if (tipo == __CONSTANTE)
            opMoveEAX(indMem, getSymbolValue(tokens.token,base,desplazamiento));
        else if(tipo == __VARIABLE)
            opMoveEAX_EDI(indMem, getSymbolValue(tokens.token,base,desplazamiento));
        else errorSemant("ese token no existe: '", tokens.token);

        opPushEAX(indMem);
        //==========================================================================

        expectativa(__IDENT, f);
    }
    else if(tokens.tokenType == __NUMERO){

        //GENERACION DE CODIGO//============================
        opMoveEAX(indMem, atoi((tokens.token).c_str()));
        opPushEAX(indMem);
        //==================================================

        expectativa(__NUMERO, f);
    }
    else if (tokens.tokenType == __PARENTESIS_L){
        expectativa(__PARENTESIS_L, f);
        expresion(f, indMem, base, desplazamiento);
        expectativa(__PARENTESIS_R, f);
    }
    else errorSintax("factor vacio.", "ident', un numero o '(");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////SIMBOLS
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void agregarSimbolo(string tipo, int base, int desplazamiento, int &varDir){
    int finTabla = base + desplazamiento;

    if(tipo == __NUMERO){
        simbTab[finTabla-1].valor = atoi((tokens.token).c_str());
        if(simbTab[finTabla-1].tipo != __CONSTANTE) errorSemant("al asignar numero a const: '", simbTab[finTabla].nombre);
        return;
    }

    for(int i = base; i <= finTabla; i++){
        if(simbTab[i].nombre == tokens.token)   errorSemant("identificador repetido: '", tokens.token);
    }

    simbTab[finTabla].nombre = tokens.token;
    simbTab[finTabla].tipo = tipo;

    if(tipo == __CONSTANTE) simbTab[finTabla].valor = 0;
    if(tipo == __VARIABLE)  simbTab[finTabla].valor = varDir;
    if(tipo == __PROCEDURE) simbTab[finTabla].valor = varDir;

}

void verificarIdentificador(int lado, int base, int desplazamiento){
    int finTabla = base + desplazamiento - 1;
    int index = -1;

    while(finTabla != -1){
        if (tokens.token == simbTab[finTabla].nombre)  index = finTabla;
        finTabla--;
    }

    if (index == -1)   errorSemant("simbolo indefinido: ", tokens.token);

    switch(lado){
        case LEFT:
            if (simbTab[index].tipo == __PROCEDURE)
            	errorSemant("falta palabra reservada 'call' antes de: '", simbTab[index].nombre);

            if (simbTab[index].tipo != __VARIABLE)
            	errorSemant("tiene que ser una variable: '", simbTab[index].nombre);
            break;
        case RIGHT:
            if (simbTab[index].tipo == __PROCEDURE)  errorSemant("no puede ser un procedure: '", simbTab[index].nombre);
            break;
        case CALL:
            if (simbTab[index].tipo != __PROCEDURE)  errorSemant("tiene que ser un procedure: '", simbTab[index].nombre);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////CODE GENERATOR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cgInit(){
    /* MS-DOS COMPATIBLE HEADER */
    memoria[0] = 0x4D; /*'M' (Magic number)*/                   memoria[2] = 0x60; // Bytes on last block
    memoria[1] = 0x5A; /*'Z'*/                                  memoria[3] = 0x01; // (1 bl. = 512 bytes)

    memoria[4] = 0x01; /*Number of blocks*/                     memoria[6] = 0x00; // Number of
    memoria[5] = 0x00; /*in the EXE file*/                      memoria[7] = 0x00; // relocation entries

    memoria[8] = 0x04; /*Size of header*/                       memoria[10] = 0x00; // Minimum extra
    memoria[9] = 0x00; /*(x 16 bytes)*/                         memoria[11] = 0x00; // paragraphs needed

    memoria[12] = 0xFF; /*Maximum extra*/                       memoria[14] = 0x00; // Initial (relative)
    memoria[13] = 0xFF; /*paragraphs needed*/                   memoria[15] = 0x00; // SS value

    memoria[16] = 0x60; /*Initial SP value*/                    memoria[18] = 0x00; // Checksum
    memoria[17] = 0x01;                                         memoria[19] = 0x00;

    memoria[20] = 0x00; /*Initial IP value*/                    memoria[22] = 0x00; // Initial (relative)
    memoria[21] = 0x00;                                         memoria[23] = 0x00; // CS value

    memoria[24] = 0x40; /*Offset of the 1st*/                   memoria[26] = 0x00; // Overlay number.
    memoria[25] = 0x00; /*relocation item*/                     memoria[27] = 0x00; // (0 = main program)

    memoria[28] = 0x00; /*Reserved word*/                       memoria[30] = 0x00; // Reserved word
    memoria[29] = 0x00;                                         memoria[31] = 0x00;

    memoria[32] = 0x00; /*Reserved word*/                       memoria[34] = 0x00; // Reserved word
    memoria[33] = 0x00;                                         memoria[35] = 0x00;

    memoria[36] = 0x00; /*OEM identifier*/                      memoria[38] = 0x00; // OEM information
    memoria[37] = 0x00;                                         memoria[39] = 0x00;

    memoria[40] = 0x00; /*Reserved word*/                       memoria[42] = 0x00; // Reserved word
    memoria[41] = 0x00;                                         memoria[43] = 0x00;

    memoria[44] = 0x00; /*Reserved word*/                       memoria[46] = 0x00; // Reserved word
    memoria[45] = 0x00;                                         memoria[47] = 0x00;

    memoria[48] = 0x00; /*Reserved word*/                       memoria[50] = 0x00; // Reserved word
    memoria[49] = 0x00;                                         memoria[51] = 0x00;

    memoria[52] = 0x00; /*Reserved word*/                       memoria[54] = 0x00; // Reserved word
    memoria[53] = 0x00;                                         memoria[55] = 0x00;

    memoria[56] = 0x00; /*Reserved word*/                       memoria[58] = 0x00; // Reserved word
    memoria[57] = 0x00;                                         memoria[59] = 0x00;

    memoria[60] = 0xA0; /*Start of the COFF*/                   memoria[69] = 0xB4; // MOV AH,09
    memoria[61] = 0x00; /*header*/                              memoria[70] = 0x09;
    memoria[62] = 0x00;                                         memoria[71] = 0xCD; // INT 21
    memoria[63] = 0x00;                                         memoria[72] = 0x21;
    memoria[64] = 0x0E; /*PUSH CS*/                             memoria[73] = 0xB8; // MOV AX,4C01
    memoria[65] = 0x1F; /*POP DS*/                              memoria[74] = 0x01;
    memoria[66] = 0xBA; /*MOV DX,000E*/                         memoria[75] = 0x4C;
    memoria[67] = 0x0E;                                         memoria[76] = 0xCD; // INT 21
    memoria[68] = 0x00;                                         memoria[77] = 0x21;

    memoria[78] = 0x54; /*'T'*/                                 memoria[110] = 0x61; // 'a'
    memoria[79] = 0x68; /*'h'*/                                 memoria[111] = 0x70; // 'p'
    memoria[80] = 0x69; /*'i'*/                                 memoria[112] = 0x70; // 'p'
    memoria[81] = 0x73; /*'s'*/                                 memoria[113] = 0x6C; // 'l'
    memoria[82] = 0x20; /*' '*/                                 memoria[114] = 0x69; // 'i'
    memoria[83] = 0x70; /*'p'*/                                 memoria[115] = 0x63; // 'c'
    memoria[84] = 0x72; /*'r'*/                                 memoria[116] = 0x61; // 'a'
    memoria[85] = 0x6F; /*'o'*/                                 memoria[117] = 0x74; // 't'
    memoria[86] = 0x67; /*'g'*/                                 memoria[118] = 0x69; // 'i'
    memoria[87] = 0x72; /*'r'*/                                 memoria[119] = 0x6F; // 'o'
    memoria[88] = 0x61; /*'a'*/                                 memoria[120] = 0x6E; // 'n'
    memoria[89] = 0x6D; /*'m'*/                                 memoria[121] = 0x2E; // '.'
    memoria[90] = 0x20; /*' '*/                                 memoria[122] = 0x20; // ' '
    memoria[91] = 0x69; /*'i'*/                                 memoria[123] = 0x49; // 'I'
    memoria[92] = 0x73; /*'s'*/                                 memoria[124] = 0x74; // 't'
    memoria[93] = 0x20; /*' '*/                                 memoria[125] = 0x20; // ' '
    memoria[94] = 0x61; /*'a'*/                                 memoria[126] = 0x63; // 'c'
    memoria[95] = 0x20; /*' '*/                                 memoria[127] = 0x61; // 'a'
    memoria[96] = 0x57; /*'W'*/                                 memoria[128] = 0x6E; // 'n'
    memoria[97] = 0x69; /*'i'*/                                 memoria[129] = 0x6E; // 'n'
    memoria[98] = 0x6E; /*'n'*/                                 memoria[130] = 0x6F; // 'o'
    memoria[99] = 0x33; /*'3'*/                                 memoria[131] = 0x74; // 't'
    memoria[100] = 0x32; /*'2'*/                                memoria[132] = 0x20; // ' '
    memoria[101] = 0x20; /*' '*/                                memoria[133] = 0x62; // 'b'
    memoria[102] = 0x63; /*'c'*/                                memoria[134] = 0x65; // 'e'
    memoria[103] = 0x6F; /*'o'*/                                memoria[135] = 0x20; // ' '
    memoria[104] = 0x6E; /*'n'*/                                memoria[136] = 0x72; // 'r'
    memoria[105] = 0x73; /*'s'*/                                memoria[137] = 0x75; // 'u'
    memoria[106] = 0x6F; /*'o'*/                                memoria[138] = 0x6E; // 'n'
    memoria[107] = 0x6C; /*'l'*/                                memoria[139] = 0x20; // ' '
    memoria[108] = 0x65; /*'e'*/
    memoria[109] = 0x20; /*' '*/

    memoria[140] = 0x75; /*'u'*/                                memoria[146] = 0x4D; // 'M'
    memoria[141] = 0x6E; /*'n'*/                                memoria[147] = 0x53; // 'S'
    memoria[142] = 0x64; /*'d'*/                                memoria[148] = 0x2D; // '-'
    memoria[143] = 0x65; /*'e'*/                                memoria[149] = 0x44; // 'D'
    memoria[144] = 0x72; /*'r'*/                                memoria[150] = 0x4F; // 'O'
    memoria[145] = 0x20; /*' '*/                                memoria[151] = 0x53; // 'S'

    memoria[152] = 0x2E; // '.'

    memoria[153] = 0x0D; // Carriage return
    memoria[154] = 0x0A; // Line feed
    memoria[155] = 0x24; // String end ('$')
    memoria[156] = 0x00;
    memoria[157] = 0x00;
    memoria[158] = 0x00;
    memoria[159] = 0x00;

    /* COFF HEADER - 8 Standard fields */
    memoria[160] = 0x50; /*'P'*/                                memoria[172] = 0x00; // Pointer to symbol
    memoria[161] = 0x45; /*'E'*/                                memoria[173] = 0x00; // table
    memoria[162] = 0x00; /*'\0'*/                               memoria[174] = 0x00; // (deprecated)
    memoria[163] = 0x00; /*'\0'*/                               memoria[175] = 0x00;

    memoria[164] = 0x4C; /*Machine:*/                           memoria[176] = 0x00; // Number of symbols
    memoria[165] = 0x01; /*>= Intel 386*/                       memoria[177] = 0x00; // (deprecated)
    memoria[166] = 0x01; /*Number of*/                          memoria[178] = 0x00;
    memoria[167] = 0x00; /*sections*/                           memoria[179] = 0x00;

    memoria[168] = 0x00; /*Time/Date stamp*/                    memoria[180] = 0xE0; // Size of optional
    memoria[169] = 0x00;                                        memoria[181] = 0x00; // header
    memoria[170] = 0x53;                                        memoria[182] = 0x02; // Characteristics:
    memoria[171] = 0x4C;                                        memoria[183] = 0x01; // 32BIT_MACHINE EXE

    /* OPTIONAL HEADER - 8 Standard fields */
    /* (For image files, it is required) */
    memoria[184] = 0x0B; // Magic number
    memoria[185] = 0x01; // (010B = PE32)
    memoria[186] = 0x01; // Maj.Linker Version
    memoria[187] = 0x00; // Min.Linker Version
    memoria[188] = 0x00; // Size of code
    memoria[189] = 0x06; // (text) section
    memoria[190] = 0x00;
    memoria[191] = 0x00;
    memoria[192] = 0x00; // Size of
    memoria[193] = 0x00; // initialized data
    memoria[194] = 0x00; // section
    memoria[195] = 0x00;
    memoria[196] = 0x00; // Size of
    memoria[197] = 0x00; // uninitialized
    memoria[198] = 0x00; // data section
    memoria[199] = 0x00;
    memoria[200] = 0x00; // Starting address
    memoria[201] = 0x15; // relative to the
    memoria[202] = 0x00; // image base
    memoria[203] = 0x00;
    memoria[204] = 0x00; // Base of code
    memoria[205] = 0x10;
    memoria[206] = 0x00;
    memoria[207] = 0x00;

    /* OPT.HEADER - 1 PE32 specific field */
    memoria[208] = 0x00; // Base of data
    memoria[209] = 0x20;
    memoria[210] = 0x00;
    memoria[211] = 0x00;

    /* OPT.HEADER - 21 Win-Specific Fields */
    memoria[212] = 0x00; // Image base
    memoria[213] = 0x00; // (Preferred
    memoria[214] = 0x40; // address of image
    memoria[215] = 0x00; // when loaded)
    memoria[216] = 0x00; // Section alignment
    memoria[217] = 0x10;
    memoria[218] = 0x00;
    memoria[219] = 0x00;
    memoria[220] = 0x00; // File alignment
    memoria[221] = 0x02; // (Default is 512)
    memoria[222] = 0x00;
    memoria[223] = 0x00;
    memoria[224] = 0x04; // Major OS version
    memoria[225] = 0x00;
    memoria[226] = 0x00; // Minor OS version
    memoria[227] = 0x00;
    memoria[228] = 0x00; // Maj. image version
    memoria[229] = 0x00;
    memoria[230] = 0x00; // Min. image version
    memoria[231] = 0x00;
    memoria[232] = 0x04; // Maj.subsystem ver.
    memoria[233] = 0x00;
    memoria[234] = 0x00; // Min.subsystem ver.
    memoria[235] = 0x00;
    memoria[236] = 0x00; // Win32 version
    memoria[237] = 0x00; // (Reserved, must
    memoria[238] = 0x00; // be zero)
    memoria[239] = 0x00;
    memoria[240] = 0x00; // Size of image
    memoria[241] = 0x20; // (It must be a
    memoria[242] = 0x00; // multiple of the
    memoria[243] = 0x00; // section alignment)
    memoria[244] = 0x00; // Size of headers
    memoria[245] = 0x02; // (rounded up to a
    memoria[246] = 0x00; // multiple of the
    memoria[247] = 0x00; // file alignment)
    memoria[248] = 0x00; // Checksum
    memoria[249] = 0x00;
    memoria[250] = 0x00;
    memoria[251] = 0x00;
    memoria[252] = 0x03; // Windows subsystem
    memoria[253] = 0x00; // (03 = console)
    memoria[254] = 0x00; // DLL characmemoria[255] = 0x00; // teristics
    memoria[256] = 0x00; // Size of stack
    memoria[257] = 0x00; // reserve
    memoria[258] = 0x10;
    memoria[259] = 0x00;
    memoria[260] = 0x00; // Size of stack
    memoria[261] = 0x10; // commit
    memoria[262] = 0x00;
    memoria[263] = 0x00;
    memoria[264] = 0x00; // Size of heap
    memoria[265] = 0x00; // reserve
    memoria[266] = 0x10;
    memoria[267] = 0x00;
    memoria[268] = 0x00; // Size of heap
    memoria[269] = 0x10; // commit
    memoria[270] = 0x00;
    memoria[271] = 0x00;
    memoria[272] = 0x00; // Loader flags
    memoria[273] = 0x00; // (Reserved, must
    memoria[274] = 0x00; // be zero)
    memoria[275] = 0x00;
    memoria[276] = 0x10; // Number of
    memoria[277] = 0x00; // relative virtual
    memoria[278] = 0x00; // addresses (RVAs)
    memoria[279] = 0x00;

    /* OPT. HEADER - 16 Data Directories */
    memoria[280] = 0x00; /*Export Table*/       memoria[288] = 0x1C; // Import Table
    memoria[281] = 0x00;                        memoria[289] = 0x10;
    memoria[282] = 0x00;                        memoria[290] = 0x00;
    memoria[283] = 0x00;                        memoria[291] = 0x00;
    memoria[284] = 0x00;                        memoria[292] = 0x28;
    memoria[285] = 0x00;                        memoria[293] = 0x00;
    memoria[286] = 0x00;                        memoria[294] = 0x00;
    memoria[287] = 0x00;                        memoria[295] = 0x00;

    memoria[296] = 0x00; /*Resource Table*/     memoria[304] = 0x00; // Exception Table
    memoria[297] = 0x00;                        memoria[305] = 0x00;
    memoria[298] = 0x00;                        memoria[306] = 0x00;
    memoria[299] = 0x00;                        memoria[307] = 0x00;
    memoria[300] = 0x00;                        memoria[308] = 0x00;
    memoria[301] = 0x00;                        memoria[309] = 0x00;
    memoria[302] = 0x00;                        memoria[310] = 0x00;
    memoria[303] = 0x00;                        memoria[311] = 0x00;

    memoria[312] = 0x00; /*Certificate Table*/  memoria[320] = 0x00; // Base Relocation Table
    memoria[313] = 0x00;                        memoria[321] = 0x00;
    memoria[314] = 0x00;                        memoria[322] = 0x00;
    memoria[315] = 0x00;                        memoria[323] = 0x00;
    memoria[316] = 0x00;                        memoria[324] = 0x00;
    memoria[317] = 0x00;                        memoria[325] = 0x00;
    memoria[318] = 0x00;                        memoria[326] = 0x00;
    memoria[319] = 0x00;                        memoria[327] = 0x00;

    memoria[328] = 0x00; /*Debug*/              memoria[336] = 0x00; // Architecture
    memoria[329] = 0x00;                        memoria[337] = 0x00;
    memoria[330] = 0x00;                        memoria[338] = 0x00;
    memoria[331] = 0x00;                        memoria[339] = 0x00;
    memoria[332] = 0x00;                        memoria[340] = 0x00;
    memoria[333] = 0x00;                        memoria[341] = 0x00;
    memoria[334] = 0x00;                        memoria[342] = 0x00;
    memoria[335] = 0x00;                        memoria[343] = 0x00;

    memoria[344] = 0x00; /*Global Ptr*/         memoria[352] = 0x00; // TLS Table
    memoria[345] = 0x00;                        memoria[353] = 0x00;
    memoria[346] = 0x00;                        memoria[354] = 0x00;
    memoria[347] = 0x00;                        memoria[355] = 0x00;
    memoria[348] = 0x00;                        memoria[356] = 0x00;
    memoria[349] = 0x00;                        memoria[357] = 0x00;
    memoria[350] = 0x00;                        memoria[358] = 0x00;
    memoria[351] = 0x00;                        memoria[359] = 0x00;

    memoria[360] = 0x00; /*Load Config Table*/  memoria[368] = 0x00; // Bound Import
    memoria[361] = 0x00;                        memoria[369] = 0x00;
    memoria[362] = 0x00;                        memoria[370] = 0x00;
    memoria[363] = 0x00;                        memoria[371] = 0x00;
    memoria[364] = 0x00;                        memoria[372] = 0x00;
    memoria[365] = 0x00;                        memoria[373] = 0x00;
    memoria[366] = 0x00;                        memoria[374] = 0x00;
    memoria[367] = 0x00;                        memoria[375] = 0x00;

    memoria[376] = 0x00; /*IAT*/                memoria[384] = 0x00; // Delay Import Descriptor
    memoria[377] = 0x10;                        memoria[385] = 0x00;
    memoria[378] = 0x00;                        memoria[386] = 0x00;
    memoria[379] = 0x00;                        memoria[387] = 0x00;
    memoria[380] = 0x1C;                        memoria[388] = 0x00;
    memoria[381] = 0x00;                        memoria[389] = 0x00;
    memoria[382] = 0x00;                        memoria[390] = 0x00;
    memoria[383] = 0x00;                        memoria[391] = 0x00;

    memoria[392] = 0x00; /*CLR Runtime Header*/ memoria[400] = 0x00; // Reserved, must be zero
    memoria[393] = 0x00;                        memoria[401] = 0x00;
    memoria[394] = 0x00;                        memoria[402] = 0x00;
    memoria[395] = 0x00;                        memoria[403] = 0x00;
    memoria[396] = 0x00;                        memoria[404] = 0x00;
    memoria[397] = 0x00;                        memoria[405] = 0x00;
    memoria[398] = 0x00;                        memoria[406] = 0x00;
    memoria[399] = 0x00;                        memoria[407] = 0x00;

    /* SECTIONS TABLE (40 bytes per entry) */
    /* FIRST ENTRY: TEXT HEADER */
    memoria[408] = 0x2E; // '.' (Name)
    memoria[409] = 0x74; // 't'
    memoria[410] = 0x65; // 'e'
    memoria[411] = 0x78; // 'x'
    memoria[412] = 0x74; // 't'
    memoria[413] = 0x00;
    memoria[414] = 0x00;
    memoria[415] = 0x00;

    memoria[416] = 0x24; /*Virtual size*/           memoria[420] = 0x00; // Virtual address
    memoria[417] = 0x05;                            memoria[421] = 0x10;
    memoria[418] = 0x00;                            memoria[422] = 0x00;
    memoria[419] = 0x00;                            memoria[423] = 0x00;

    memoria[424] = 0x00; /*Size of raw data*/       memoria[428] = 0x00; // Pointer to raw data
    memoria[425] = 0x06;                            memoria[429] = 0x02;
    memoria[426] = 0x00;                            memoria[430] = 0x00;
    memoria[427] = 0x00;                            memoria[431] = 0x00;

    memoria[432] = 0x00; /*Pointer to relocations*/ memoria[436] = 0x00; // Pointer to line numbers
    memoria[433] = 0x00;                            memoria[437] = 0x00;
    memoria[434] = 0x00;                            memoria[438] = 0x00;
    memoria[435] = 0x00;                            memoria[439] = 0x00;

    memoria[440] = 0x00; /*Number of relocations*/  memoria[442] = 0x00; // Number of line numbers
    memoria[441] = 0x00;                            memoria[443] = 0x00;

    memoria[444] = 0x20; // Characteristics
    memoria[445] = 0x00; // (Readable,
    memoria[446] = 0x00; // Writeable &
    memoria[447] = 0xE0; // Executable)

    // las posiciones 448-511 deberán rellenarse con ceros,
    // para que la sección text comience en la posición 512
    for (int i = 448; i <= 511; i++)
        memoria[i] = 0x00;

    // rutinas de E/S, de 512 a 1791
    memoria[512] = 0x6E;
    memoria[513] = 0x10;
    memoria[514] = 0x00;
    memoria[515] = 0x00;
    memoria[516] = 0x7C;
    memoria[517] = 0x10;
    memoria[518] = 0x00;
    memoria[519] = 0x00;
    memoria[520] = 0x8C;
    memoria[521] = 0x10;
    memoria[522] = 0x00;
    memoria[523] = 0x00;
    memoria[524] = 0x98;
    memoria[525] = 0x10;
    memoria[526] = 0x00;
    memoria[527] = 0x00;
    memoria[528] = 0xA4;
    memoria[529] = 0x10;
    memoria[530] = 0x00;
    memoria[531] = 0x00;
    memoria[532] = 0xB6;
    memoria[533] = 0x10;
    memoria[534] = 0x00;
    memoria[535] = 0x00;
    memoria[536] = 0x00;
    memoria[537] = 0x00;
    memoria[538] = 0x00;
    memoria[539] = 0x00;
    memoria[540] = 0x52;
    memoria[541] = 0x10;
    memoria[542] = 0x00;
    memoria[543] = 0x00;
    memoria[544] = 0x00;
    memoria[545] = 0x00;
    memoria[546] = 0x00;
    memoria[547] = 0x00;
    memoria[548] = 0x00;
    memoria[549] = 0x00;
    memoria[550] = 0x00;
    memoria[551] = 0x00;
    memoria[552] = 0x44;
    memoria[553] = 0x10;
    memoria[554] = 0x00;
    memoria[555] = 0x00;
    memoria[556] = 0x00;
    memoria[557] = 0x10;
    memoria[558] = 0x00;
    memoria[559] = 0x00;
    memoria[560] = 0x00;
    memoria[561] = 0x00;
    memoria[562] = 0x00;
    memoria[563] = 0x00;
    memoria[564] = 0x00;
    memoria[565] = 0x00;
    memoria[566] = 0x00;
    memoria[567] = 0x00;
    memoria[568] = 0x00;
    memoria[569] = 0x00;
    memoria[570] = 0x00;
    memoria[571] = 0x00;
    memoria[572] = 0x00;
    memoria[573] = 0x00;
    memoria[574] = 0x00;
    memoria[575] = 0x00;
    memoria[576] = 0x00;
    memoria[577] = 0x00;
    memoria[578] = 0x00;
    memoria[579] = 0x00;
    memoria[580] = 0x4B;
    memoria[581] = 0x45;
    memoria[582] = 0x52;
    memoria[583] = 0x4E;
    memoria[584] = 0x45;
    memoria[585] = 0x4C;
    memoria[586] = 0x33;
    memoria[587] = 0x32;
    memoria[588] = 0x2E;
    memoria[589] = 0x64;
    memoria[590] = 0x6C;
    memoria[591] = 0x6C;
    memoria[592] = 0x00;
    memoria[593] = 0x00;
    memoria[594] = 0x6E;
    memoria[595] = 0x10;
    memoria[596] = 0x00;
    memoria[597] = 0x00;
    memoria[598] = 0x7C;
    memoria[599] = 0x10;
    memoria[600] = 0x00;
    memoria[601] = 0x00;
    memoria[602] = 0x8C;
    memoria[603] = 0x10;
    memoria[604] = 0x00;
    memoria[605] = 0x00;
    memoria[606] = 0x98;
    memoria[607] = 0x10;
    memoria[608] = 0x00;
    memoria[609] = 0x00;
    memoria[610] = 0xA4;
    memoria[611] = 0x10;
    memoria[612] = 0x00;
    memoria[613] = 0x00;
    memoria[614] = 0xB6;
    memoria[615] = 0x10;
    memoria[616] = 0x00;
    memoria[617] = 0x00;
    memoria[618] = 0x00;
    memoria[619] = 0x00;
    memoria[620] = 0x00;
    memoria[621] = 0x00;
    memoria[622] = 0x00;
    memoria[623] = 0x00;
    memoria[624] = 0x45;
    memoria[625] = 0x78;
    memoria[626] = 0x69;
    memoria[627] = 0x74;
    memoria[628] = 0x50;
    memoria[629] = 0x72;
    memoria[630] = 0x6F;
    memoria[631] = 0x63;
    memoria[632] = 0x65;
    memoria[633] = 0x73;
    memoria[634] = 0x73;
    memoria[635] = 0x00;
    memoria[636] = 0x00;
    memoria[637] = 0x00;
    memoria[638] = 0x47;
    memoria[639] = 0x65;
    memoria[640] = 0x74;
    memoria[641] = 0x53;
    memoria[642] = 0x74;
    memoria[643] = 0x64;
    memoria[644] = 0x48;
    memoria[645] = 0x61;
    memoria[646] = 0x6E;
    memoria[647] = 0x64;
    memoria[648] = 0x6C;
    memoria[649] = 0x65;
    memoria[650] = 0x00;
    memoria[651] = 0x00;
    memoria[652] = 0x00;
    memoria[653] = 0x00;
    memoria[654] = 0x52;
    memoria[655] = 0x65;
    memoria[656] = 0x61;
    memoria[657] = 0x64;
    memoria[658] = 0x46;
    memoria[659] = 0x69;
    memoria[660] = 0x6C;
    memoria[661] = 0x65;
    memoria[662] = 0x00;
    memoria[663] = 0x00;
    memoria[664] = 0x00;
    memoria[665] = 0x00;
    memoria[666] = 0x57;
    memoria[667] = 0x72;
    memoria[668] = 0x69;
    memoria[669] = 0x74;
    memoria[670] = 0x65;
    memoria[671] = 0x46;
    memoria[672] = 0x69;
    memoria[673] = 0x6C;
    memoria[674] = 0x65;
    memoria[675] = 0x00;
    memoria[676] = 0x00;
    memoria[677] = 0x00;
    memoria[678] = 0x47;
    memoria[679] = 0x65;
    memoria[680] = 0x74;
    memoria[681] = 0x43;
    memoria[682] = 0x6F;
    memoria[683] = 0x6E;
    memoria[684] = 0x73;
    memoria[685] = 0x6F;
    memoria[686] = 0x6C;
    memoria[687] = 0x65;
    memoria[688] = 0x4D;
    memoria[689] = 0x6F;
    memoria[690] = 0x64;
    memoria[691] = 0x65;
    memoria[692] = 0x00;
    memoria[693] = 0x00;
    memoria[694] = 0x00;
    memoria[695] = 0x00;
    memoria[696] = 0x53;
    memoria[697] = 0x65;
    memoria[698] = 0x74;
    memoria[699] = 0x43;
    memoria[700] = 0x6F;
    memoria[701] = 0x6E;
    memoria[702] = 0x73;
    memoria[703] = 0x6F;
    memoria[704] = 0x6C;
    memoria[705] = 0x65;
    memoria[706] = 0x4D;
    memoria[707] = 0x6F;
    memoria[708] = 0x64;
    memoria[709] = 0x65;
    memoria[710] = 0x00;
    memoria[711] = 0x00;
    memoria[712] = 0x00;
    memoria[713] = 0x00;
    memoria[714] = 0x00;
    memoria[715] = 0x00;
    memoria[716] = 0x00;
    memoria[717] = 0x00;
    memoria[718] = 0x00;
    memoria[719] = 0x00;
    memoria[720] = 0x50;
    memoria[721] = 0xA2;
    memoria[722] = 0x1C;
    memoria[723] = 0x11;
    memoria[724] = 0x40;
    memoria[725] = 0x00;
    memoria[726] = 0x31;
    memoria[727] = 0xC0;
    memoria[728] = 0x03;
    memoria[729] = 0x05;
    memoria[730] = 0x2C;
    memoria[731] = 0x11;
    memoria[732] = 0x40;
    memoria[733] = 0x00;
    memoria[734] = 0x75;
    memoria[735] = 0x0D;
    memoria[736] = 0x6A;
    memoria[737] = 0xF5;
    memoria[738] = 0xFF;
    memoria[739] = 0x15;
    memoria[740] = 0x04;
    memoria[741] = 0x10;
    memoria[742] = 0x40;
    memoria[743] = 0x00;
    memoria[744] = 0xA3;
    memoria[745] = 0x2C;
    memoria[746] = 0x11;
    memoria[747] = 0x40;
    memoria[748] = 0x00;
    memoria[749] = 0x6A;
    memoria[750] = 0x00;
    memoria[751] = 0x68;
    memoria[752] = 0x30;
    memoria[753] = 0x11;
    memoria[754] = 0x40;
    memoria[755] = 0x00;
    memoria[756] = 0x6A;
    memoria[757] = 0x01;
    memoria[758] = 0x68;
    memoria[759] = 0x1C;
    memoria[760] = 0x11;
    memoria[761] = 0x40;
    memoria[762] = 0x00;
    memoria[763] = 0x50;
    memoria[764] = 0xFF;
    memoria[765] = 0x15;
    memoria[766] = 0x0C;
    memoria[767] = 0x10;
    memoria[768] = 0x40;
    memoria[769] = 0x00;
    memoria[770] = 0x09;
    memoria[771] = 0xC0;
    memoria[772] = 0x75;
    memoria[773] = 0x08;
    memoria[774] = 0x6A;
    memoria[775] = 0x00;
    memoria[776] = 0xFF;
    memoria[777] = 0x15;
    memoria[778] = 0x00;
    memoria[779] = 0x10;
    memoria[780] = 0x40;
    memoria[781] = 0x00;
    memoria[782] = 0x81;
    memoria[783] = 0x3D;
    memoria[784] = 0x30;
    memoria[785] = 0x11;
    memoria[786] = 0x40;
    memoria[787] = 0x00;
    memoria[788] = 0x01;
    memoria[789] = 0x00;
    memoria[790] = 0x00;
    memoria[791] = 0x00;
    memoria[792] = 0x75;
    memoria[793] = 0xEC;
    memoria[794] = 0x58;
    memoria[795] = 0xC3;
    memoria[796] = 0x00;
    memoria[797] = 0x57;
    memoria[798] = 0x72;
    memoria[799] = 0x69;
    memoria[800] = 0x74;
    memoria[801] = 0x65;
    memoria[802] = 0x20;
    memoria[803] = 0x65;
    memoria[804] = 0x72;
    memoria[805] = 0x72;
    memoria[806] = 0x6F;
    memoria[807] = 0x72;
    memoria[808] = 0x00;
    memoria[809] = 0x00;
    memoria[810] = 0x00;
    memoria[811] = 0x00;
    memoria[812] = 0x00;
    memoria[813] = 0x00;
    memoria[814] = 0x00;
    memoria[815] = 0x00;
    memoria[816] = 0x00;
    memoria[817] = 0x00;
    memoria[818] = 0x00;
    memoria[819] = 0x00;
    memoria[820] = 0x00;
    memoria[821] = 0x00;
    memoria[822] = 0x00;
    memoria[823] = 0x00;
    memoria[824] = 0x00;
    memoria[825] = 0x00;
    memoria[826] = 0x00;
    memoria[827] = 0x00;
    memoria[828] = 0x00;
    memoria[829] = 0x00;
    memoria[830] = 0x00;
    memoria[831] = 0x00;
    memoria[832] = 0x60;
    memoria[833] = 0x31;
    memoria[834] = 0xC0;
    memoria[835] = 0x03;
    memoria[836] = 0x05;
    memoria[837] = 0xCC;
    memoria[838] = 0x11;
    memoria[839] = 0x40;
    memoria[840] = 0x00;
    memoria[841] = 0x75;
    memoria[842] = 0x37;
    memoria[843] = 0x6A;
    memoria[844] = 0xF6;
    memoria[845] = 0xFF;
    memoria[846] = 0x15;
    memoria[847] = 0x04;
    memoria[848] = 0x10;
    memoria[849] = 0x40;
    memoria[850] = 0x00;
    memoria[851] = 0xA3;
    memoria[852] = 0xCC;
    memoria[853] = 0x11;
    memoria[854] = 0x40;
    memoria[855] = 0x00;
    memoria[856] = 0x68;
    memoria[857] = 0xD0;
    memoria[858] = 0x11;
    memoria[859] = 0x40;
    memoria[860] = 0x00;
    memoria[861] = 0x50;
    memoria[862] = 0xFF;
    memoria[863] = 0x15;
    memoria[864] = 0x10;
    memoria[865] = 0x10;
    memoria[866] = 0x40;
    memoria[867] = 0x00;
    memoria[868] = 0x80;
    memoria[869] = 0x25;
    memoria[870] = 0xD0;
    memoria[871] = 0x11;
    memoria[872] = 0x40;
    memoria[873] = 0x00;
    memoria[874] = 0xF9;
    memoria[875] = 0xFF;
    memoria[876] = 0x35;
    memoria[877] = 0xD0;
    memoria[878] = 0x11;
    memoria[879] = 0x40;
    memoria[880] = 0x00;
    memoria[881] = 0xFF;
    memoria[882] = 0x35;
    memoria[883] = 0xCC;
    memoria[884] = 0x11;
    memoria[885] = 0x40;
    memoria[886] = 0x00;
    memoria[887] = 0xFF;
    memoria[888] = 0x15;
    memoria[889] = 0x14;
    memoria[890] = 0x10;
    memoria[891] = 0x40;
    memoria[892] = 0x00;
    memoria[893] = 0xA1;
    memoria[894] = 0xCC;
    memoria[895] = 0x11;
    memoria[896] = 0x40;
    memoria[897] = 0x00;
    memoria[898] = 0x6A;
    memoria[899] = 0x00;
    memoria[900] = 0x68;
    memoria[901] = 0xD4;
    memoria[902] = 0x11;
    memoria[903] = 0x40;
    memoria[904] = 0x00;
    memoria[905] = 0x6A;
    memoria[906] = 0x01;
    memoria[907] = 0x68;
    memoria[908] = 0xBE;
    memoria[909] = 0x11;
    memoria[910] = 0x40;
    memoria[911] = 0x00;
    memoria[912] = 0x50;
    memoria[913] = 0xFF;
    memoria[914] = 0x15;
    memoria[915] = 0x08;
    memoria[916] = 0x10;
    memoria[917] = 0x40;
    memoria[918] = 0x00;
    memoria[919] = 0x09;
    memoria[920] = 0xC0;
    memoria[921] = 0x61;
    memoria[922] = 0x90;
    memoria[923] = 0x75;
    memoria[924] = 0x08;
    memoria[925] = 0x6A;
    memoria[926] = 0x00;
    memoria[927] = 0xFF;
    memoria[928] = 0x15;
    memoria[929] = 0x00;
    memoria[930] = 0x10;
    memoria[931] = 0x40;
    memoria[932] = 0x00;
    memoria[933] = 0x0F;
    memoria[934] = 0xB6;
    memoria[935] = 0x05;
    memoria[936] = 0xBE;
    memoria[937] = 0x11;
    memoria[938] = 0x40;
    memoria[939] = 0x00;
    memoria[940] = 0x81;
    memoria[941] = 0x3D;
    memoria[942] = 0xD4;
    memoria[943] = 0x11;
    memoria[944] = 0x40;
    memoria[945] = 0x00;
    memoria[946] = 0x01;
    memoria[947] = 0x00;
    memoria[948] = 0x00;
    memoria[949] = 0x00;
    memoria[950] = 0x74;
    memoria[951] = 0x05;
    memoria[952] = 0xB8;
    memoria[953] = 0xFF;
    memoria[954] = 0xFF;
    memoria[955] = 0xFF;
    memoria[956] = 0xFF;
    memoria[957] = 0xC3;
    memoria[958] = 0x00;
    memoria[959] = 0x52;
    memoria[960] = 0x65;
    memoria[961] = 0x61;
    memoria[962] = 0x64;
    memoria[963] = 0x20;
    memoria[964] = 0x65;
    memoria[965] = 0x72;
    memoria[966] = 0x72;
    memoria[967] = 0x6F;
    memoria[968] = 0x72;
    memoria[969] = 0x00;
    memoria[970] = 0x00;
    memoria[971] = 0x00;
    memoria[972] = 0x00;
    memoria[973] = 0x00;
    memoria[974] = 0x00;
    memoria[975] = 0x00;
    memoria[976] = 0x00;
    memoria[977] = 0x00;
    memoria[978] = 0x00;
    memoria[979] = 0x00;
    memoria[980] = 0x00;
    memoria[981] = 0x00;
    memoria[982] = 0x00;
    memoria[983] = 0x00;
    memoria[984] = 0x00;
    memoria[985] = 0x00;
    memoria[986] = 0x00;
    memoria[987] = 0x00;
    memoria[988] = 0x00;
    memoria[989] = 0x00;
    memoria[990] = 0x00;
    memoria[991] = 0x00;
    memoria[992] = 0x60;
    memoria[993] = 0x89;
    memoria[994] = 0xC6;
    memoria[995] = 0x30;
    memoria[996] = 0xC0;
    memoria[997] = 0x02;
    memoria[998] = 0x06;
    memoria[999] = 0x74;
    memoria[1000] = 0x8;
    memoria[1001] = 0x46;
    memoria[1002] = 0xE8;
    memoria[1003] = 0xE1;
    memoria[1004] = 0xFE;
    memoria[1005] = 0xFF;
    memoria[1006] = 0xFF;
    memoria[1007] = 0xEB;
    memoria[1008] = 0xF2;
    memoria[1009] = 0x61;
    memoria[1010] = 0x90;
    memoria[1011] = 0xC3;
    memoria[1012] = 0x00;
    memoria[1013] = 0x00;
    memoria[1014] = 0x00;
    memoria[1015] = 0x00;
    memoria[1016] = 0x00;
    memoria[1017] = 0x00;
    memoria[1018] = 0x00;
    memoria[1019] = 0x00;
    memoria[1020] = 0x00;
    memoria[1021] = 0x00;
    memoria[1022] = 0x00;
    memoria[1023] = 0x00;
    memoria[1024] = 0x04;
    memoria[1025] = 0x30;
    memoria[1026] = 0xE8;
    memoria[1027] = 0xC9;
    memoria[1028] = 0xFE;
    memoria[1029] = 0xFF;
    memoria[1030] = 0xFF;
    memoria[1031] = 0xC3;
    memoria[1032] = 0x00;
    memoria[1033] = 0x00;
    memoria[1034] = 0x00;
    memoria[1035] = 0x00;
    memoria[1036] = 0x00;
    memoria[1037] = 0x00;
    memoria[1038] = 0x00;
    memoria[1039] = 0x00;
    memoria[1040] = 0xB0;
    memoria[1041] = 0x0D;
    memoria[1042] = 0xE8;
    memoria[1043] = 0xB9;
    memoria[1044] = 0xFE;
    memoria[1045] = 0xFF;
    memoria[1046] = 0xFF;
    memoria[1047] = 0xB0;
    memoria[1048] = 0x0A;
    memoria[1049] = 0xE8;
    memoria[1050] = 0xB2;
    memoria[1051] = 0xFE;
    memoria[1052] = 0xFF;
    memoria[1053] = 0xFF;
    memoria[1054] = 0xC3;
    memoria[1055] = 0x00;
    memoria[1056] = 0x3D;
    memoria[1057] = 0x00;
    memoria[1058] = 0x00;
    memoria[1059] = 0x00;
    memoria[1060] = 0x80;
    memoria[1061] = 0x75;
    memoria[1062] = 0x4E;
    memoria[1063] = 0xB0;
    memoria[1064] = 0x2D;
    memoria[1065] = 0xE8;
    memoria[1066] = 0xA2;
    memoria[1067] = 0xFE;
    memoria[1068] = 0xFF;
    memoria[1069] = 0xFF;
    memoria[1070] = 0xB0;
    memoria[1071] = 0x02;
    memoria[1072] = 0xE8;
    memoria[1073] = 0xCB;
    memoria[1074] = 0xFF;
    memoria[1075] = 0xFF;
    memoria[1076] = 0xFF;
    memoria[1077] = 0xB0;
    memoria[1078] = 0x01;
    memoria[1079] = 0xE8;
    memoria[1080] = 0xC4;
    memoria[1081] = 0xFF;
    memoria[1082] = 0xFF;
    memoria[1083] = 0xFF;
    memoria[1084] = 0xB0;
    memoria[1085] = 0x04;
    memoria[1086] = 0xE8;
    memoria[1087] = 0xBD;
    memoria[1088] = 0xFF;
    memoria[1089] = 0xFF;
    memoria[1090] = 0xFF;
    memoria[1091] = 0xB0;
    memoria[1092] = 0x07;
    memoria[1093] = 0xE8;
    memoria[1094] = 0xB6;
    memoria[1095] = 0xFF;
    memoria[1096] = 0xFF;
    memoria[1097] = 0xFF;
    memoria[1098] = 0xB0;
    memoria[1099] = 0x04;
    memoria[1100] = 0xE8;
    memoria[1101] = 0xAF;
    memoria[1102] = 0xFF;
    memoria[1103] = 0xFF;
    memoria[1104] = 0xFF;
    memoria[1105] = 0xB0;
    memoria[1106] = 0x08;
    memoria[1107] = 0xE8;
    memoria[1108] = 0xA8;
    memoria[1109] = 0xFF;
    memoria[1110] = 0xFF;
    memoria[1111] = 0xFF;
    memoria[1112] = 0xB0;
    memoria[1113] = 0x03;
    memoria[1114] = 0xE8;
    memoria[1115] = 0xA1;
    memoria[1116] = 0xFF;
    memoria[1117] = 0xFF;
    memoria[1118] = 0xFF;
    memoria[1119] = 0xB0;
    memoria[1120] = 0x06;
    memoria[1121] = 0xE8;
    memoria[1122] = 0x9A;
    memoria[1123] = 0xFF;
    memoria[1124] = 0xFF;
    memoria[1125] = 0xFF;
    memoria[1126] = 0xB0;
    memoria[1127] = 0x04;
    memoria[1128] = 0xE8;
    memoria[1129] = 0x93;
    memoria[1130] = 0xFF;
    memoria[1131] = 0xFF;
    memoria[1132] = 0xFF;
    memoria[1133] = 0xB0;
    memoria[1134] = 0x08;
    memoria[1135] = 0xE8;
    memoria[1136] = 0x8C;
    memoria[1137] = 0xFF;
    memoria[1138] = 0xFF;
    memoria[1139] = 0xFF;
    memoria[1140] = 0xC3;
    memoria[1141] = 0x3D;
    memoria[1142] = 0x00;
    memoria[1143] = 0x00;
    memoria[1144] = 0x00;
    memoria[1145] = 0x00;
    memoria[1146] = 0x7D;
    memoria[1147] = 0x0B;
    memoria[1148] = 0x50;
    memoria[1149] = 0xB0;
    memoria[1150] = 0x2D;
    memoria[1151] = 0xE8;
    memoria[1152] = 0x4C;
    memoria[1153] = 0xFE;
    memoria[1154] = 0xFF;
    memoria[1155] = 0xFF;
    memoria[1156] = 0x58;
    memoria[1157] = 0xF7;
    memoria[1158] = 0xD8;
    memoria[1159] = 0x3D;
    memoria[1160] = 0x0A;
    memoria[1161] = 0x00;
    memoria[1162] = 0x00;
    memoria[1163] = 0x00;
    memoria[1164] = 0x0F;
    memoria[1165] = 0x8C;
    memoria[1166] = 0xEF;
    memoria[1167] = 0x00;
    memoria[1168] = 0x00;
    memoria[1169] = 0x00;
    memoria[1170] = 0x3D;
    memoria[1171] = 0x64;
    memoria[1172] = 0x00;
    memoria[1173] = 0x00;
    memoria[1174] = 0x00;
    memoria[1175] = 0x0F;
    memoria[1176] = 0x8C;
    memoria[1177] = 0xD1;
    memoria[1178] = 0x00;
    memoria[1179] = 0x00;
    memoria[1180] = 0x00;
    memoria[1181] = 0x3D;
    memoria[1182] = 0xE8;
    memoria[1183] = 0x03;
    memoria[1184] = 0x00;
    memoria[1185] = 0x00;
    memoria[1186] = 0x0F;
    memoria[1187] = 0x8C;
    memoria[1188] = 0xB3;
    memoria[1189] = 0x00;
    memoria[1190] = 0x00;
    memoria[1191] = 0x00;
    memoria[1192] = 0x3D;
    memoria[1193] = 0x10;
    memoria[1194] = 0x27;
    memoria[1195] = 0x00;
    memoria[1196] = 0x00;
    memoria[1197] = 0x0F;
    memoria[1198] = 0x8C;
    memoria[1199] = 0x95;
    memoria[1200] = 0x00;
    memoria[1201] = 0x00;
    memoria[1202] = 0x00;
    memoria[1203] = 0x3D;
    memoria[1204] = 0xA0;
    memoria[1205] = 0x86;
    memoria[1206] = 0x01;
    memoria[1207] = 0x00;
    memoria[1208] = 0x7C;
    memoria[1209] = 0x7B;
    memoria[1210] = 0x3D;
    memoria[1211] = 0x40;
    memoria[1212] = 0x42;
    memoria[1213] = 0x0F;
    memoria[1214] = 0x00;
    memoria[1215] = 0x7C;
    memoria[1216] = 0x61;
    memoria[1217] = 0x3D;
    memoria[1218] = 0x80;
    memoria[1219] = 0x96;
    memoria[1220] = 0x98;
    memoria[1221] = 0x00;
    memoria[1222] = 0x7C;
    memoria[1223] = 0x47;
    memoria[1224] = 0x3D;
    memoria[1225] = 0x00;
    memoria[1226] = 0xE1;
    memoria[1227] = 0xF5;
    memoria[1228] = 0x05;
    memoria[1229] = 0x7C;
    memoria[1230] = 0x2D;
    memoria[1231] = 0x3D;
    memoria[1232] = 0x00;
    memoria[1233] = 0xCA;
    memoria[1234] = 0x9A;
    memoria[1235] = 0x3B;
    memoria[1236] = 0x7C;
    memoria[1237] = 0x13;
    memoria[1238] = 0xBA;
    memoria[1239] = 0x00;
    memoria[1240] = 0x00;
    memoria[1241] = 0x00;
    memoria[1242] = 0x00;
    memoria[1243] = 0xBB;
    memoria[1244] = 0x00;
    memoria[1245] = 0xCA;
    memoria[1246] = 0x9A;
    memoria[1247] = 0x3B;
    memoria[1248] = 0xF7;
    memoria[1249] = 0xFB;
    memoria[1250] = 0x52;
    memoria[1251] = 0xE8;
    memoria[1252] = 0x18;
    memoria[1253] = 0xFF;
    memoria[1254] = 0xFF;
    memoria[1255] = 0xFF;
    memoria[1256] = 0x58;
    memoria[1257] = 0xBA;
    memoria[1258] = 0x00;
    memoria[1259] = 0x00;
    memoria[1260] = 0x00;
    memoria[1261] = 0x00;
    memoria[1262] = 0xBB;
    memoria[1263] = 0x00;
    memoria[1264] = 0xE1;
    memoria[1265] = 0xF5;
    memoria[1266] = 0x05;
    memoria[1267] = 0xF7;
    memoria[1268] = 0xFB;
    memoria[1269] = 0x52;
    memoria[1270] = 0xE8;
    memoria[1271] = 0x05;
    memoria[1272] = 0xFF;
    memoria[1273] = 0xFF;
    memoria[1274] = 0xFF;
    memoria[1275] = 0x58;
    memoria[1276] = 0xBA;
    memoria[1277] = 0x00;
    memoria[1278] = 0x00;
    memoria[1279] = 0x00;
    memoria[1280] = 0x00;
    memoria[1281] = 0xBB;
    memoria[1282] = 0x80;
    memoria[1283] = 0x96;
    memoria[1284] = 0x98;
    memoria[1285] = 0x00;
    memoria[1286] = 0xF7;
    memoria[1287] = 0xFB;
    memoria[1288] = 0x52;
    memoria[1289] = 0xE8;
    memoria[1290] = 0xF2;
    memoria[1291] = 0xFE;
    memoria[1292] = 0xFF;
    memoria[1293] = 0xFF;
    memoria[1294] = 0x58;
    memoria[1295] = 0xBA;
    memoria[1296] = 0x00;
    memoria[1297] = 0x00;
    memoria[1298] = 0x00;
    memoria[1299] = 0x00;
    memoria[1300] = 0xBB;
    memoria[1301] = 0x40;
    memoria[1302] = 0x42;
    memoria[1303] = 0x0F;
    memoria[1304] = 0x00;
    memoria[1305] = 0xF7;
    memoria[1306] = 0xFB;
    memoria[1307] = 0x52;
    memoria[1308] = 0xE8;
    memoria[1309] = 0xDF;
    memoria[1310] = 0xFE;
    memoria[1311] = 0xFF;
    memoria[1312] = 0xFF;
    memoria[1313] = 0x58;
    memoria[1314] = 0xBA;
    memoria[1315] = 0x00;
    memoria[1316] = 0x00;
    memoria[1317] = 0x00;
    memoria[1318] = 0x00;
    memoria[1319] = 0xBB;
    memoria[1320] = 0xA0;
    memoria[1321] = 0x86;
    memoria[1322] = 0x01;
    memoria[1323] = 0x00;
    memoria[1324] = 0xF7;
    memoria[1325] = 0xFB;
    memoria[1326] = 0x52;
    memoria[1327] = 0xE8;
    memoria[1328] = 0xCC;
    memoria[1329] = 0xFE;
    memoria[1330] = 0xFF;
    memoria[1331] = 0xFF;
    memoria[1332] = 0x58;
    memoria[1333] = 0xBA;
    memoria[1334] = 0x00;
    memoria[1335] = 0x00;
    memoria[1336] = 0x00;
    memoria[1337] = 0x00;
    memoria[1338] = 0xBB;
    memoria[1339] = 0x10;
    memoria[1340] = 0x27;
    memoria[1341] = 0x00;
    memoria[1342] = 0x00;
    memoria[1343] = 0xF7;
    memoria[1344] = 0xFB;
    memoria[1345] = 0x52;
    memoria[1346] = 0xE8;
    memoria[1347] = 0xB9;
    memoria[1348] = 0xFE;
    memoria[1349] = 0xFF;
    memoria[1350] = 0xFF;
    memoria[1351] = 0x58;
    memoria[1352] = 0xBA;
    memoria[1353] = 0x00;
    memoria[1354] = 0x00;
    memoria[1355] = 0x00;
    memoria[1356] = 0x00;
    memoria[1357] = 0xBB;
    memoria[1358] = 0xE8;
    memoria[1359] = 0x03;
    memoria[1360] = 0x00;
    memoria[1361] = 0x00;
    memoria[1362] = 0xF7;
    memoria[1363] = 0xFB;
    memoria[1364] = 0x52;
    memoria[1365] = 0xE8;
    memoria[1366] = 0xA6;
    memoria[1367] = 0xFE;
    memoria[1368] = 0xFF;
    memoria[1369] = 0xFF;
    memoria[1370] = 0x58;
    memoria[1371] = 0xBA;
    memoria[1372] = 0x00;
    memoria[1373] = 0x00;
    memoria[1374] = 0x00;
    memoria[1375] = 0x00;
    memoria[1376] = 0xBB;
    memoria[1377] = 0x64;
    memoria[1378] = 0x00;
    memoria[1379] = 0x00;
    memoria[1380] = 0x00;
    memoria[1381] = 0xF7;
    memoria[1382] = 0xFB;
    memoria[1383] = 0x52;
    memoria[1384] = 0xE8;
    memoria[1385] = 0x93;
    memoria[1386] = 0xFE;
    memoria[1387] = 0xFF;
    memoria[1388] = 0xFF;
    memoria[1389] = 0x58;
    memoria[1390] = 0xBA;
    memoria[1391] = 0x00;
    memoria[1392] = 0x00;
    memoria[1393] = 0x00;
    memoria[1394] = 0x00;
    memoria[1395] = 0xBB;
    memoria[1396] = 0x0A;
    memoria[1397] = 0x00;
    memoria[1398] = 0x00;
    memoria[1399] = 0x00;
    memoria[1400] = 0xF7;
    memoria[1401] = 0xFB;
    memoria[1402] = 0x52;
    memoria[1403] = 0xE8;
    memoria[1404] = 0x80;
    memoria[1405] = 0xFE;
    memoria[1406] = 0xFF;
    memoria[1407] = 0xFF;
    memoria[1408] = 0x58;
    memoria[1409] = 0xE8;
    memoria[1410] = 0x7A;
    memoria[1411] = 0xFE;
    memoria[1412] = 0xFF;
    memoria[1413] = 0xFF;
    memoria[1414] = 0xC3;
    memoria[1415] = 0x00;
    memoria[1416] = 0xFF;
    memoria[1417] = 0x15;
    memoria[1418] = 0x00;
    memoria[1419] = 0x10;
    memoria[1420] = 0x40;
    memoria[1421] = 0x00;
    memoria[1422] = 0x00;
    memoria[1423] = 0x00;
    memoria[1424] = 0xB9;
    memoria[1425] = 0x00;
    memoria[1426] = 0x00;
    memoria[1427] = 0x00;
    memoria[1428] = 0x00;
    memoria[1429] = 0xB3;
    memoria[1430] = 0x03;
    memoria[1431] = 0x51;
    memoria[1432] = 0x53;
    memoria[1433] = 0xE8;
    memoria[1434] = 0xA2;
    memoria[1435] = 0xFD;
    memoria[1436] = 0xFF;
    memoria[1437] = 0xFF;
    memoria[1438] = 0x5B;
    memoria[1439] = 0x59;
    memoria[1440] = 0x3C;
    memoria[1441] = 0x0D;
    memoria[1442] = 0x0F;
    memoria[1443] = 0x84;
    memoria[1444] = 0x34;
    memoria[1445] = 0x01;
    memoria[1446] = 0x00;
    memoria[1447] = 0x00;
    memoria[1448] = 0x3C;
    memoria[1449] = 0x08;
    memoria[1450] = 0x0F;
    memoria[1451] = 0x84;
    memoria[1452] = 0x94;
    memoria[1453] = 0x00;
    memoria[1454] = 0x00;
    memoria[1455] = 0x00;
    memoria[1456] = 0x3C;
    memoria[1457] = 0x2D;
    memoria[1458] = 0x0F;
    memoria[1459] = 0x84;
    memoria[1460] = 0x09;
    memoria[1461] = 0x01;
    memoria[1462] = 0x00;
    memoria[1463] = 0x00;
    memoria[1464] = 0x3C;
    memoria[1465] = 0x30;
    memoria[1466] = 0x7C;
    memoria[1467] = 0xDB;
    memoria[1468] = 0x3C;
    memoria[1469] = 0x39;
    memoria[1470] = 0x7F;
    memoria[1471] = 0xD7;
    memoria[1472] = 0x2C;
    memoria[1473] = 0x30;
    memoria[1474] = 0x80;
    memoria[1475] = 0xFB;
    memoria[1476] = 0x00;
    memoria[1477] = 0x74;
    memoria[1478] = 0xD0;
    memoria[1479] = 0x80;
    memoria[1480] = 0xFB;
    memoria[1481] = 0x02;
    memoria[1482] = 0x75;
    memoria[1483] = 0x0C;
    memoria[1484] = 0x81;
    memoria[1485] = 0xF9;
    memoria[1486] = 0x00;
    memoria[1487] = 0x00;
    memoria[1488] = 0x00;
    memoria[1489] = 0x00;
    memoria[1490] = 0x75;
    memoria[1491] = 0x04;
    memoria[1492] = 0x3C;
    memoria[1493] = 0x00;
    memoria[1494] = 0x74;
    memoria[1495] = 0xBF;
    memoria[1496] = 0x80;
    memoria[1497] = 0xFB;
    memoria[1498] = 0x03;
    memoria[1499] = 0x75;
    memoria[1500] = 0x0A;
    memoria[1501] = 0x3C;
    memoria[1502] = 0x00;
    memoria[1503] = 0x75;
    memoria[1504] = 0x04;
    memoria[1505] = 0xB3;
    memoria[1506] = 0x00;
    memoria[1507] = 0xEB;
    memoria[1508] = 0x02;
    memoria[1509] = 0xB3;
    memoria[1510] = 0x01;
    memoria[1511] = 0x81;
    memoria[1512] = 0xF9;
    memoria[1513] = 0xCC;
    memoria[1514] = 0xCC;
    memoria[1515] = 0xCC;
    memoria[1516] = 0x0C;
    memoria[1517] = 0x7F;
    memoria[1518] = 0xA8;
    memoria[1519] = 0x81;
    memoria[1520] = 0xF9;
    memoria[1521] = 0x34;
    memoria[1522] = 0x33;
    memoria[1523] = 0x33;
    memoria[1524] = 0xF3;
    memoria[1525] = 0x7C;
    memoria[1526] = 0xA0;
    memoria[1527] = 0x88;
    memoria[1528] = 0xC7;
    memoria[1529] = 0xB8;
    memoria[1530] = 0x0A;
    memoria[1531] = 0x00;
    memoria[1532] = 0x00;
    memoria[1533] = 0x00;
    memoria[1534] = 0xF7;
    memoria[1535] = 0xE9;
    memoria[1536] = 0x3D;
    memoria[1537] = 0x08;
    memoria[1538] = 0x00;
    memoria[1539] = 0x00;
    memoria[1540] = 0x80;
    memoria[1541] = 0x74;
    memoria[1542] = 0x11;
    memoria[1543] = 0x3D;
    memoria[1544] = 0xF8;
    memoria[1545] = 0xFF;
    memoria[1546] = 0xFF;
    memoria[1547] = 0x7F;
    memoria[1548] = 0x75;
    memoria[1549] = 0x13;
    memoria[1550] = 0x80;
    memoria[1551] = 0xFF;
    memoria[1552] = 0x07;
    memoria[1553] = 0x7E;
    memoria[1554] = 0x0E;
    memoria[1555] = 0xE9;
    memoria[1556] = 0x7F;
    memoria[1557] = 0xFF;
    memoria[1558] = 0xFF;
    memoria[1559] = 0xFF;
    memoria[1560] = 0x80;
    memoria[1561] = 0xFF;
    memoria[1562] = 0x08;
    memoria[1563] = 0x0F;
    memoria[1564] = 0x8F;
    memoria[1565] = 0x76;
    memoria[1566] = 0xFF;
    memoria[1567] = 0xFF;
    memoria[1568] = 0xFF;
    memoria[1569] = 0xB9;
    memoria[1570] = 0x00;
    memoria[1571] = 0x00;
    memoria[1572] = 0x00;
    memoria[1573] = 0x00;
    memoria[1574] = 0x88;
    memoria[1575] = 0xF9;
    memoria[1576] = 0x80;
    memoria[1577] = 0xFB;
    memoria[1578] = 0x02;
    memoria[1579] = 0x74;
    memoria[1580] = 0x04;
    memoria[1581] = 0x01;
    memoria[1582] = 0xC1;
    memoria[1583] = 0xEB;
    memoria[1584] = 0x03;
    memoria[1585] = 0x29;
    memoria[1586] = 0xC8;
    memoria[1587] = 0x91;
    memoria[1588] = 0x88;
    memoria[1589] = 0xF8;
    memoria[1590] = 0x51;
    memoria[1591] = 0x53;
    memoria[1592] = 0xE8;
    memoria[1593] = 0xC3;
    memoria[1594] = 0xFD;
    memoria[1595] = 0xFF;
    memoria[1596] = 0xFF;
    memoria[1597] = 0x5B;
    memoria[1598] = 0x59;
    memoria[1599] = 0xE9;
    memoria[1600] = 0x53;
    memoria[1601] = 0xFF;
    memoria[1602] = 0xFF;
    memoria[1603] = 0xFF;
    memoria[1604] = 0x80;
    memoria[1605] = 0xFB;
    memoria[1606] = 0x03;
    memoria[1607] = 0x0F;
    memoria[1608] = 0x84;
    memoria[1609] = 0x4A;
    memoria[1610] = 0xFF;
    memoria[1611] = 0xFF;
    memoria[1612] = 0xFF;
    memoria[1613] = 0x51;
    memoria[1614] = 0x53;
    memoria[1615] = 0xB0;
    memoria[1616] = 0x08;
    memoria[1617] = 0xE8;
    memoria[1618] = 0x7A;
    memoria[1619] = 0xFC;
    memoria[1620] = 0xFF;
    memoria[1621] = 0xFF;
    memoria[1622] = 0xB0;
    memoria[1623] = 0x20;
    memoria[1624] = 0xE8;
    memoria[1625] = 0x73;
    memoria[1626] = 0xFC;
    memoria[1627] = 0xFF;
    memoria[1628] = 0xFF;
    memoria[1629] = 0xB0;
    memoria[1630] = 0x08;
    memoria[1631] = 0xE8;
    memoria[1632] = 0x6C;
    memoria[1633] = 0xFC;
    memoria[1634] = 0xFF;
    memoria[1635] = 0xFF;
    memoria[1636] = 0x5B;
    memoria[1637] = 0x59;
    memoria[1638] = 0x80;
    memoria[1639] = 0xFB;
    memoria[1640] = 0x00;
    memoria[1641] = 0x75;
    memoria[1642] = 0x07;
    memoria[1643] = 0xB3;
    memoria[1644] = 0x03;
    memoria[1645] = 0xE9;
    memoria[1646] = 0x25;
    memoria[1647] = 0xFF;
    memoria[1648] = 0xFF;
    memoria[1649] = 0xFF;
    memoria[1650] = 0x80;
    memoria[1651] = 0xFB;
    memoria[1652] = 0x02;
    memoria[1653] = 0x75;
    memoria[1654] = 0x0F;
    memoria[1655] = 0x81;
    memoria[1656] = 0xF9;
    memoria[1657] = 0x00;
    memoria[1658] = 0x00;
    memoria[1659] = 0x00;
    memoria[1660] = 0x00;
    memoria[1661] = 0x75;
    memoria[1662] = 0x07;
    memoria[1663] = 0xB3;
    memoria[1664] = 0x03;
    memoria[1665] = 0xE9;
    memoria[1666] = 0x11;
    memoria[1667] = 0xFF;
    memoria[1668] = 0xFF;
    memoria[1669] = 0xFF;
    memoria[1670] = 0x89;
    memoria[1671] = 0xC8;
    memoria[1672] = 0xB9;
    memoria[1673] = 0x0A;
    memoria[1674] = 0x00;
    memoria[1675] = 0x00;
    memoria[1676] = 0x00;
    memoria[1677] = 0xBA;
    memoria[1678] = 0x00;
    memoria[1679] = 0x00;
    memoria[1680] = 0x00;
    memoria[1681] = 0x00;
    memoria[1682] = 0x3D;
    memoria[1683] = 0x00;
    memoria[1684] = 0x00;
    memoria[1685] = 0x00;
    memoria[1686] = 0x00;
    memoria[1687] = 0x7D;
    memoria[1688] = 0x08;
    memoria[1689] = 0xF7;
    memoria[1690] = 0xD8;
    memoria[1691] = 0xF7;
    memoria[1692] = 0xF9;
    memoria[1693] = 0xF7;
    memoria[1694] = 0xD8;
    memoria[1695] = 0xEB;
    memoria[1696] = 0x02;
    memoria[1697] = 0xF7;
    memoria[1698] = 0xF9;
    memoria[1699] = 0x89;
    memoria[1700] = 0xC1;
    memoria[1701] = 0x81;
    memoria[1702] = 0xF9;
    memoria[1703] = 0x00;
    memoria[1704] = 0x00;
    memoria[1705] = 0x00;
    memoria[1706] = 0x00;
    memoria[1707] = 0x0F;
    memoria[1708] = 0x85;
    memoria[1709] = 0xE6;
    memoria[1710] = 0xFE;
    memoria[1711] = 0xFF;
    memoria[1712] = 0xFF;
    memoria[1713] = 0x80;
    memoria[1714] = 0xFB;
    memoria[1715] = 0x02;
    memoria[1716] = 0x0F;
    memoria[1717] = 0x84;
    memoria[1718] = 0xDD;
    memoria[1719] = 0xFE;
    memoria[1720] = 0xFF;
    memoria[1721] = 0xFF;
    memoria[1722] = 0xB3;
    memoria[1723] = 0x03;
    memoria[1724] = 0xE9;
    memoria[1725] = 0xD6;
    memoria[1726] = 0xFE;
    memoria[1727] = 0xFF;
    memoria[1728] = 0xFF;
    memoria[1729] = 0x80;
    memoria[1730] = 0xFB;
    memoria[1731] = 0x03;
    memoria[1732] = 0x0F;
    memoria[1733] = 0x85;
    memoria[1734] = 0xCD;
    memoria[1735] = 0xFE;
    memoria[1736] = 0xFF;
    memoria[1737] = 0xFF;
    memoria[1738] = 0xB0;
    memoria[1739] = 0x2D;
    memoria[1740] = 0x51;
    memoria[1741] = 0x53;
    memoria[1742] = 0xE8;
    memoria[1743] = 0xFD;
    memoria[1744] = 0xFB;
    memoria[1745] = 0xFF;
    memoria[1746] = 0xFF;
    memoria[1747] = 0x5B;
    memoria[1748] = 0x59;
    memoria[1749] = 0xB3;
    memoria[1750] = 0x02;
    memoria[1751] = 0xE9;
    memoria[1752] = 0xBB;
    memoria[1753] = 0xFE;
    memoria[1754] = 0xFF;
    memoria[1755] = 0xFF;
    memoria[1756] = 0x80;
    memoria[1757] = 0xFB;
    memoria[1758] = 0x03;
    memoria[1759] = 0x0F;
    memoria[1760] = 0x84;
    memoria[1761] = 0xB2;
    memoria[1762] = 0xFE;
    memoria[1763] = 0xFF;
    memoria[1764] = 0xFF;
    memoria[1765] = 0x80;
    memoria[1766] = 0xFB;
    memoria[1767] = 0x02;
    memoria[1768] = 0x75;
    memoria[1769] = 0x0C;
    memoria[1770] = 0x81;
    memoria[1771] = 0xF9;
    memoria[1772] = 0x00;
    memoria[1773] = 0x00;
    memoria[1774] = 0x00;
    memoria[1775] = 0x00;
    memoria[1776] = 0x0F;
    memoria[1777] = 0x84;
    memoria[1778] = 0xA1;
    memoria[1779] = 0xFE;
    memoria[1780] = 0xFF;
    memoria[1781] = 0xFF;
    memoria[1782] = 0x51;
    memoria[1783] = 0xE8;
    memoria[1784] = 0x14;
    memoria[1785] = 0xFD;
    memoria[1786] = 0xFF;
    memoria[1787] = 0xFF;
    memoria[1788] = 0x59;
    memoria[1789] = 0x89;
    memoria[1790] = 0xC8;
    memoria[1791] = 0xC3;

    // MOV EDI, 00 00 00 00 ->  APUNTA A LA TABLA DE SIMBOLOS, SE ARREGLA DESPUES
    memoria[1792] = 0xBF;
    for (int i = 1793; i <= 1796; i++)
        memoria[i] = 0x00;
}

void inttchar(long long int val, arr4Bytes a){
    if (val < 0){
        val = __LONG_MAX + val;
    }
    for(int i = 0; i < 4; i++){
        char resp = (char)(val%256);
        a[i] = resp;
        val/=256;
    }
}

void mem0(int &indMem, int cant){
    for (int i = 0; i < cant; i++)
        memoria[indMem++] = 0x00;
}

void opMoveEDI(int &indMem, int val){
    memoria[indMem++] = 0xBF;

    arr4Bytes d;
    inttchar(val, d);
    for(int i = 0; i < 4; i++){
        memoria[indMem++] = d[i];
    }
}

void opMoveEAX_EDI(int &indMem, int val){
    memoria[indMem++] = 0x8B;
    memoria[indMem++] = 0x87;

    arr4Bytes d;
    inttchar(val, d);
    for(int i = 0; i < 4; i++){
        memoria[indMem++] = d[i];
    }
}

void opMoveEDI_EAX(int &indMem, int val){
    memoria[indMem++] = 0x89;
    memoria[indMem++] = 0x87;

    arr4Bytes d;
    inttchar(val, d);
    for(int i = 0; i < 4; i++){
        memoria[indMem++] = d[i];
    }
}

void opMoveEAX(int &indMem, int val){
    memoria[indMem++] = 0xB8;

    arr4Bytes d;
    inttchar(val, d);
    for(int i = 0; i < 4; i++){
        memoria[indMem++] = d[i];
    }
}

void opXCHG(int &indMem){
    memoria[indMem++] = 0x93;
}

void opPushEAX(int &indMem){
    memoria[indMem++] = 0x50;
}

void opPopEAX(int &indMem){
    memoria[indMem++] = 0x58;
}

void opPopEBX(int &indMem){
    memoria[indMem++] = 0x5B;
}

void opADD(int &indMem){
    memoria[indMem++] = 0x01;
    memoria[indMem++] = 0xD8;
}

void opSUBB(int &indMem){
    memoria[indMem++] = 0x29;
    memoria[indMem++] = 0xD8;
}

void opIMUL(int &indMem){
    memoria[indMem++] = 0xF7;
    memoria[indMem++] = 0xEB;
}

void opIDIV(int &indMem){
    memoria[indMem++] = 0xF7;
    memoria[indMem++] = 0xFB;
}

void opCDQ(int &indMem){
    memoria[indMem++] = 0x99;
}

void opNEG(int &indMem){
    memoria[indMem++] = 0xF7;
    memoria[indMem++] = 0xD8;
}

void opTEST(int &indMem, char val){
    memoria[indMem++] = 0xA8;
    memoria[indMem++] = val;
}

void opCMP(int &indMem){
    memoria[indMem++] = 0x39;
    memoria[indMem++] = 0xC3;
}

void opJE(int &indMem){
    memoria[indMem++] = 0x74;
    memoria[indMem++] = 0x05;
}

void opJNE(int &indMem){
    memoria[indMem++] = 0x75;
    memoria[indMem++] = 0x05;
}

void opJL(int &indMem){
    memoria[indMem++] = 0x7C;
    memoria[indMem++] = 0x05;
}

void opJLE(int &indMem){
    memoria[indMem++] = 0x7E;
    memoria[indMem++] = 0x05;
}

void opJG(int &indMem){
    memoria[indMem++] = 0x7F;
    memoria[indMem++] = 0x05;
}

void opJGE(int &indMem){
    memoria[indMem++] = 0x7D;
    memoria[indMem++] = 0x05;
}

void opJPO(int &indMem){
    memoria[indMem++] = 0x7B;
    memoria[indMem++] = 0x05;
}

void opJMP(int &indMem, int val){
    if(val < 0) val -= 5;
    memoria[indMem++] = 0xE9;

    arr4Bytes d;
    inttchar(val, d);
    for(int i = 0; i < 4; i++){
        memoria[indMem++] = d[i];
    }
}

void opCALL(int &indMem, int val){
    if(val < 0) val -= 5;
    memoria[indMem++] = 0xE8;

    arr4Bytes d;
    inttchar(val, d);
    for(int i = 0; i < 4; i++){
        memoria[indMem++] = d[i];
    }
}

void opRET(int &indMem){
    memoria[indMem++] = 0xC3;
}

int getSymbolValue(string token, int base, int desplazamiento){
    int finTabla = base + desplazamiento - 1;

    while(finTabla != -1){
        if (token == simbTab[finTabla].nombre)  return simbTab[finTabla].valor;
        finTabla--;
    }
    return -1;
}

string getSymbolType(string token, int base, int desplazamiento){
    int finTabla = base + desplazamiento - 1;

    while(finTabla != -1){
        if (token == simbTab[finTabla].nombre)  return simbTab[finTabla].tipo;
        finTabla--;
    }
    return "";
}

void arreglarMem4bytes(int indMemArreglar, int val){
    arr4Bytes d;
    inttchar(val, d);
    for(int i = 0; i < 4; i++){
        memoria[indMemArreglar++] = d[i];
    }
}

int byte4toint(char ms, char um, char lm, char ls){
    int aux = (ls<<0),
        aux2 = (lm<<8),
        aux3 = (um<<16),
        aux4 = (ms<<24);
    return (aux + aux2 + aux3 + aux4);
}
