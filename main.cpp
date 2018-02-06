//
// Created by SHU TAN on 2/6/18.
//

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

int linecount;
int machine_size = 199;
int moduleamount;
int modulecount;
int use_amount;
int real_value;
string all_str;

map<string, int> symboltable;
map<int, int> base_address;
map<string, int> def_exceeds;
map<string, int> def_position;//string is the symbol, int is the position of module where it is defined;

int basecount = 0;
int text_size;

bool Is2nd;

vector<string> redefined;//to save the error checking result: the symbol are multiple defined;
vector<string> uselist;//to save the whole symbol was used in this module
vector<string> module_use;//to save the symbol use list for current module;
vector<int> module_size;//to save the size of each module;

//used to do error checking6, the symboal appearing in a use list of a module, but not actually used in this module
map<string, int> actual_use;//string is the symbol, int is the position of the module

vector<string> nouse_symbol;
vector<int> nouse_symbol_position;

//

map <string, int > check_resulet;// to save the error checking result: symbol value > module size;
map <string, int> check_result1;//to save the error checking result: symbol is defined, but never used;



void read(string filepath){
    ifstream input;
    input.open(filepath);
    string str;
    while( getline(input, str) ) {
        if (str == "") {
            break;
        }
        else {
            all_str.append(str + " ");
        }
    }
    input.close();
}

bool error_checking2(string symbol){
    map<string, int>:: iterator itself = symboltable.begin();
    while(itself != symboltable.end()){
        if( symbol == itself -> first){
            redefined.push_back(symbol);
            return true;//this symbool was multiple defined;
        }
        itself++;
    }
    return false;// this symbol has not been defined
}//multiple defined

bool error_checking(int size){ //the size of symbol > the size of module
    map<string, int>::iterator itself = def_exceeds.begin();
    while(itself != def_exceeds.end()){
        string symbol = itself -> first;
        //cout << itself -> first <<itself -> second << endl;
        int temp_value = itself -> second;
        if(temp_value > size){
            check_resulet[symbol] = modulecount;
            symboltable[symbol] = 0;
            //cout << symbol << " " << symboltable[symbol];
            return true;//the siuze if symbol > the size of the module
        }
        itself++;
    }
    return false;

}

//symbol is defined, but never used;

void error_checking1(){
    map<string, int>::iterator itself = symboltable.begin();
    while(itself != symboltable.end()){
        string symbol = itself -> first;
        int valid = 0;

        for(int i = 0; i < uselist.size(); i++){
            if(symbol == uselist[i]){
                valid = 1;
                break;
            }
        }
        if(valid == 0){
            check_result1[symbol] = def_position[symbol];
        }
        itself++;
    }
}

//absolute address > machine size
bool error_checking7(int digits){
    int address = digits % 1000;
    if(address > machine_size){
        return true;//the error exists: absolute address > machine size
    }
    return false;
}

//relative address > the size of the module
bool error_checking8(int digits){
    int opcode = digits / 1000;
    int address = digits % 1000;
    int modulesize = module_size[modulecount];
    if(address > modulesize){
        return true;// error 8 exists: relative address > module size
    }
    return false;
}

//whether the used symbol is defined
bool error_checking3(string symbol){
    map<string, int>::iterator itself = symboltable.begin();
    while(itself != symboltable.end()){
        if(symbol == itself -> first){
            return false;//error3 does not exist: the used symbol is defined
        }
        itself++;
    }
    return true;//error3 exists.
}

//a symbol appears in a use list, but not actually used
void error_checking6(string symbol){
    map<string, int>::iterator itself = actual_use.begin();
    while(itself != actual_use.end()){
        if(symbol == itself -> first){
            actual_use.erase(symbol);
            break;
        }
        itself++;
    }
}

void error_checking6_2(){
    map<string, int>::iterator itself = actual_use.begin();
    while(itself != actual_use.end()){
        nouse_symbol.push_back(itself->first);
        nouse_symbol_position.push_back(itself->second);
        itself++;
    }
}

string E_address(int digits){
    int address = digits % 1000;
    int opcode = digits / 1000;
    int use_size = module_use.size();
    string message;

    //to do the error checking 5;
    if(address > use_size - 1){
        real_value = digits;
        message = "  Error: External address exceeds length of use list; treated as immediate.";
    }

    else{
        string symbol = module_use[address];
        error_checking6(symbol);
        if(error_checking3(symbol)){
            real_value = opcode * 1000;
            message = " Error: " + symbol + " is not defined; zero used.";
        }
        else{
            real_value = opcode * 1000 + symboltable[symbol];
        }
    }
    return message;

}

string digits_cal(string type, int digits){
    string message;
    int opcode;
    int address;
    if( type == "A"){
        if(error_checking7(digits)){
            opcode = digits / 1000;
            real_value = opcode * 1000;
            message = " Error: Absolute address exceeds machine size; zero used. ";
        }
        else{
            real_value = digits;
        }
    }
    else if ( type == "R"){
        if(error_checking8(digits)){
            opcode = digits / 1000;
            real_value = opcode * 1000;
            message = " Error: Relative address exceeds module size; zero used. ";
        }
        else{
            real_value = digits + base_address[modulecount];
        }
    }
    else if (type == "E"){
        E_address(digits);
        message = E_address(digits);

    }
    else{
        real_value = digits;// I type
    }

    return message;


}


void read_def(istringstream& strm) {
    string symbol;
    int value;
    int def_amount;

    strm >> def_amount;
    //cout << def_amount << endl;
    if(!Is2nd){
        while (def_amount != 0) {
            strm >> symbol;
            strm >> value;
            if (!error_checking2(symbol)) {

                symboltable[symbol] = value;
                //cout << symbol <<"  " << value << endl;
                def_exceeds[symbol] = value;
                def_position[symbol] = modulecount;
            }
            def_amount = def_amount - 1;
        }
    }
    else if(Is2nd){
        while(def_amount != 0){
            strm >> symbol;
            strm >>value;
            def_amount = def_amount - 1;
        }
    }
}

void read_use(istringstream& strm){
    strm >> use_amount;

    if(use_amount != 0){
        string symbol;
        for(int index = 0; index < use_amount; index++){
            strm >> symbol;
            uselist.push_back(symbol);
            if(Is2nd){
                module_use.push_back(symbol);
                actual_use[symbol] = modulecount;
            }
        }

    }
}

void read_text(istringstream& strm){
    strm >> text_size;
    int text_size2 = text_size;

    if(!Is2nd){
        base_address[modulecount] = basecount;
        basecount = basecount + text_size;
        module_size.push_back(text_size);
        error_checking(text_size);
        def_exceeds.clear();
    }
    while(text_size != 0){
        //cout << text_size << endl;
        string type;
        int digits;

        strm >> type >> digits;
        if (Is2nd){
            digits_cal(type, digits);
            cout << linecount << ": " << real_value << digits_cal(type, digits) << endl;
            linecount++;
        }


        text_size = text_size - 1;
    }
}

void cal_symboltable(int modulecount){

    map<string, int>::iterator itself = symboltable.begin();
    while(itself != symboltable.end()){
        string symbol = itself -> first;
        int value = itself -> second;
        //cout << symbol << "--" << symboltable[symbol] << "--" << base_address[def_position[symbol]] << endl;

        value = symboltable[symbol] + base_address[def_position[symbol]];
        symboltable[symbol] = value;
        itself++;
    }

}

void print_symboltable() {
    cout << "Symbol Table" << endl;
    map<string, int>::iterator itself = symboltable.begin();
    while (itself != symboltable.end()) {
        string symbol = itself->first;
        int value = itself->second;
        vector<string>::iterator tocheck;
        cout << symbol << " = " << value;
        itself++;
        vector<string>::iterator it;
        it = find(redefined.begin(), redefined.end(), symbol);
        if (it != redefined.end()) {
            cout << " Error: This variable is multiply defined; first value used." << endl;
        } else {
            cout << endl;
        }
    }
}



//to print the result of error checking: size of symbol > size of the module
void print_def_over(){
    map<string, int> :: iterator itself = check_resulet.begin();
    while(itself != check_resulet.end()){
        string symbol = itself -> first;
        int module = itself -> second;
        cout << "Error: In module " << module << " the def of " << symbol << " exceeds the module size; zero (relative) used." << endl;
        itself++;
    }
}

//to print error1:symbol is defined but never used
void print_error1(){
    map<string, int>::iterator itself = check_result1.begin();
    while(itself != check_result1.end()){
        cout << "Warning: " << itself->first << " was defined in module " << itself->second << " but never used." << endl;
        itself++;
    }
}
void print_error6(){
    int end = nouse_symbol.size();
    for( int i=0; i < end; i++){
        cout << "Warning: In module " << nouse_symbol_position[i]<<" " << nouse_symbol[i] << " appeared in the use list but was not actually used." << endl;
    }
}

void first_pass(istringstream& strm){
    for(modulecount = 0; modulecount < moduleamount; modulecount++){

        read_def(strm);
        read_use(strm);
        read_text(strm);
    }
    cal_symboltable(modulecount);
    print_symboltable();
}

void second_pass(istringstream& strm){
    for(modulecount = 0; modulecount < moduleamount; modulecount++){
        //cout << modulecount << endl;
        read_def(strm);
        read_use(strm);
        read_text(strm);

        error_checking6_2();
        module_use.clear();
        actual_use.erase(actual_use.begin(), actual_use.end());
    }
    print_error6();
    error_checking1();
    print_error1();
    print_def_over();

    //test for symbol defined but never used
    /*
    map<string, int>::iterator itself = symboltable.begin();
    int test_size = uselist.size();
    while(itself != symboltable.end()){
        for(int i =0; i < test_size; i++){
            if(itself -> first == uselist[i]){
                break;
            }
            cout << itself->first << "--";
        }
        itself++;
    }
    */
}

int main(int argc, char* argv[]) {
    Is2nd = false;
    string filepath;
    //cout << "Please enter the filepath: ";
    //std::cin>>filepath;
    filepath = "/Users/shutan/Desktop/operating-system/Lab1/Inputs/input8.txt";
    read(filepath);
    istringstream iss(all_str);
    iss >> moduleamount;
    first_pass(iss);

    Is2nd = true;
    cout << "Memory Map" << endl;
    read(filepath);
    iss.clear();
    iss.str(all_str);
    iss >> moduleamount;
    second_pass(iss);




    return 0;
}
