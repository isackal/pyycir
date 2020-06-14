#include <iostream>
#include <vector>
#include <string>

#define BUFFER_SIZE 10000
#define circuit_init(name, size) Circuit name(size); workingCircuit=&name
#define set_circuit(name) workingCircuit = &name
#define def(name,tp,size) LogicGate& name = workingCircuit->add(tp, size)
#define sut(x) std::cout << x
#define print(x) std::cout << x << std::endl;
#define def_circuit(c,n,s) Circuit n(s)
#define cexec workingCircuit->run()
#define _IN(x) workingCircuit->input(x)
#define _OUT(x) workingCircuit->output(x)

enum eErrorCodes{
    ERROR_UNEXISTING_KEY,
    ERROR_NOT_EDITABLE
};

enum eLogicType{
    SWITCH,
    CONSTANT,
    BUFFER,
    NOT,
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR,
    CIRCUIT
};

enum eIOType{
    IO_NONE,
    IO_INPUT,
    IO_OUTPUT
};

class LogicGate
{
    protected:
        LogicGate** _input;
        int _inputSize;
        bool _output;
        bool loopBreak;
        int inpAt;
        bool isIO;
        eLogicType logicType;
        friend class Circuit;
    public:
        bool executed;
        void* master;
        LogicGate* clone, *parent;
        unsigned int refNumber;
        int refPin;
        eIOType ioType;
        LogicGate(eLogicType tp){
            logicType = tp;
            _input = nullptr;
            _inputSize = 0;
            inpAt = 0;
            _output = false;
            master = nullptr;
            refNumber = 0;
            refPin = -1;
            isIO = false;
            clone = nullptr;
            parent = nullptr;
            ioType = IO_NONE;
        }
        ~LogicGate(){
            if (_input){
                delete [] _input;
            }
        }
        bool value(){
            return _output;
        }
        void toggle(){
            _output = !_output;
        }
        void setValue(bool val){
            _output = val;
        }
        void setInputSize(int __inputSize){
            if (__inputSize > 0){
                _input = new LogicGate*[__inputSize];
                for (int i=0; i < __inputSize; i++){
                    _input[i] = nullptr;
                }
                _inputSize = __inputSize;
            }
        }
        bool availableInput(){
            return inpAt < _inputSize;
        }

        void addInput(LogicGate* g){
            if (inpAt < _inputSize){
                _input[inpAt++] = g;
            }
        }
        void connect(LogicGate* g){
            g->addInput(this);
        }

        virtual void reset(){
            executed = false;
        }
        virtual void setInput(unsigned int x, LogicGate* y){
            if (x < _inputSize){
                _input[x] = y;
            }
        }
        virtual bool transform(){
            bool ret = _output;
            switch (logicType)
            {
            case SWITCH:
                ret = _output;
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                }
                sut("C");
                sut(ret);
                break;
            case CONSTANT:
                ret = _output;
                sut("C");
                sut(ret);
                break;
            case BUFFER: {
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                }
                sut("B(" << _input[0]->_output << ")" << ret);
                break;
            }
            case NOT: {
                if (_input && _input[0]){
                    ret = !_input[0]->_output;
                }
                sut("!(" << _input[0]->_output << ")" << ret);
                break;
            }
            case AND: {
                sut("&(");
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                    sut(_input[0]->_output);
                    for (int i=1; ret && (i<_inputSize); i++){
                        ret = ret && _input[i]->_output;
                        sut(_input[i]->_output);
                    }
                    sut(")" << ret);
                }
                break;
            }

            case NAND: {
                sut("!&(");
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                    sut(_input[0]->_output);
                    for (int i=1; ret && (i<_inputSize); i++){
                        sut(_input[i]->_output);
                        ret = ret && _input[i]->_output;
                    }
                    ret = !ret;
                    sut(")" << ret);
                }
                break;
            }

            case OR: {
                sut("|(");
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                    sut(_input[0]->_output);
                    for (int i=1; !ret && (i<_inputSize); i++){
                        ret = ret || _input[i]->_output;
                        sut(_input[i]->_output);
                    }
                }
                sut(")" << ret);
                break;
            }

            case NOR: {
                sut("!|(");
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                    sut(_input[0]->_output);
                    for (int i=1; !ret && (i<_inputSize); i++){
                        ret = ret || _input[i]->_output;
                        sut(_input[i]->_output);
                    }
                    ret = !ret;
                    sut(")" << ret);
                }
                break;
            }

            case XOR: {
                sut("x(");
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                    sut(_input[0]->_output);
                    for (int i=1; i<_inputSize; i++){
                        ret = _input[i]->_output ? !ret : ret;
                        sut(_input[i]->_output);
                    }
                    sut(")"<<ret);
                }
                break;
            }

            case XNOR: {
                sut("!x");
                if (_input && _input[0]){
                    ret = _input[0]->_output;
                    sut(_input[0]->_output);
                    for (int i=1; i<_inputSize; i++){
                        ret = _input[i]->_output ? !ret : ret;
                        sut(_input[i]->_output);
                    }
                    ret = !ret;
                    sut(")" << ret);
                }
                break;
            }
            default:
                break;
            }
            sut("\n");
            return ret;
        }
        virtual bool exec();
        virtual LogicGate* copy(){
            LogicGate* g = new LogicGate(logicType);
            g->setInputSize(_inputSize);
            g->_output = _output;
            clone = g;
            g->parent = this;
            return g;
        }
        void operator >>(LogicGate& other){
            connect(&other);
        }
        virtual void display(){
            print(inpAt);
            print(_input[0]);
            if (logicType == BUFFER){
                print("type: output");
            }else if (logicType == SWITCH){
                print("type: input");
            }else {
                print("type: gate");
            }
        }
        void exportInput(std::string _name);
        void exportOutput(std::string _name);
};

class Circuit : public LogicGate
{
    protected:
        LogicGate** buffer;
        std::vector<LogicGate*> inputs;
        std::vector<std::string> inputNames;
        std::vector<LogicGate*> outputs;
        std::vector<std::string> outputNames;
        int bufferSize;
        int at;
        bool editable;
        bool executed;
    public:
        Circuit() : LogicGate(CIRCUIT)
        {
            at=0;
            bufferSize=0;
            buffer = nullptr;
            editable = true;
            executed = true;
        }
        Circuit(int _bufferSize) : LogicGate(CIRCUIT)
        {
            at=0;
            init(_bufferSize);
            editable = true;
            executed = true;
        }
        ~Circuit(){
            if (buffer){
                for (int i=0; i<at; i++){
                    delete buffer[i];
                }
                delete [] buffer;
            }
        }

        LogicGate* copy(){
            Circuit* c = new Circuit(at);
            clone = c;
            c->parent = this;
            c->at = at;
            LogicGate* g, *_g, *_cl;
            Circuit* q, *_q;
            int i, j, ref, sz;

            print("Copy whole buffer:");
            // Copy each LogicGate / Circuit aka. copy whole Buffer:
            for (i=0; i<at; i++){
                g = buffer[i]->copy();
                _cl = buffer[i];
                //printbuffer[i]->clone);
                g->master = c;
                g->refNumber = i;
                c->buffer[i] = g;
            }

            // Copy inputs
            print("Copy inputs:");
            for (i=0; i<at; i++){
                g = buffer[i];
                _g = c->buffer[i];
                if (g->logicType == CIRCUIT){
                    q = (Circuit*) g;
                    _q = (Circuit*) _g;
                    sz = q->inputs.size();
                    for (j=0; j<sz; j++){
                        _q->inputs[j]->_input[0] = q->inputs[j]->_input[0]->clone;
                    }
                }else{
                    _g->inpAt = g->inpAt;
                    for (j=0; j<g->inpAt; j++){
                        _g->_input[j] = g->_input[j]->clone;
                    }
                }
                print("!");
            }

            print("Copy exportations:");
            sz = inputs.size();
            for (i=0; i<sz; i++){
                ref = inputs[i]->refNumber;
                c->buffer[ref]->exportInput( inputNames[i] );
            }
            sz = outputs.size();
            for (i=0; i<sz; i++){
                ref = outputs[i]->refNumber;
                c->buffer[ref]->exportOutput( outputNames[i] );
            }
            c->editable = false;
            return c;
        }
        /*
        LogicGate* copy(){
            Circuit* c = new Circuit(at);
            LogicGate* g, *_g;
            Circuit* q, *_q;
            int i, j, ref, sz;
            // Copy each LogicGate / Circuit aka. copy whole Buffer:
            print("#1");
            for (i=0; i<at; i++){
                g = buffer[i]->copy();
                g->master = c;
                g->refNumber = i;
                c->buffer[i] = g;
            }
            // Copy the logic gates inputs:
            print("#2");
            for (i=0; i<at;i++){
                print("#2" << i);
                g = buffer[i];
                _g = c->buffer[i];
                print("#2.1" << i);
                if (g->logicType == CIRCUIT){
                    print("2.3");
                    q = (Circuit*) g;
                    _q = (Circuit*) _g;
                    sz = q->inputs.size();
                    for (int j=0; j<sz; j++){  // For each input buffer g, _g
                        g = q->inputs[j];
                        _g = _q->inputs[j];
                        LogicGate* t = g->_input[0];  // t is input into the input buffer (buffer gate)
                        if (t->isIO){
                            Circuit* tm = (Circuit*) t->master;
                            Circuit* _tm =(Circuit*) c->buffer[tm->refNumber];
                            _g->_input[0] = _tm->outputs[t->refPin];
                        }else{
                            _g->_input[0] = c->buffer[t->refNumber];
                        }
                    }
                }else{
                    print("2.2");
                    for (j=0; j < g->inpAt; j++){
                        print("2.2.1");
                        LogicGate* t = g->_input[j];  // t is input into the input buffer (buffer gate)
                        print("2.2.2");
                        if (t->isIO){
                            Circuit* tm = (Circuit*) t->master;
                            Circuit* _tm =(Circuit*) c->buffer[tm->refNumber];
                            _g->_input[j] = _tm->outputs[t->refPin];
                        }else{
                            _g->_input[j] = c->buffer[t->refNumber];
                        }
                    }
                }
            }
            print("#3");
            // Export input and outputs.
            sz = inputs.size();
            for (i=0; i<sz; i++){
                ref = inputs[i]->refNumber;
                c->buffer[ref]->exportInput( inputNames[i] );
            }
            sz = outputs.size();
            for (i=0; i<sz; i++){
                ref = outputs[i]->refNumber;
                c->buffer[ref]->exportOutput( outputNames[i] );
            }
            c->editable = false;
            return c;
        }
        */

        Circuit& make(){
            Circuit* c = (Circuit*)copy();
            return *c;
        }

        void reset(){
            executed = false;
            for (int i=0; i<at; i++){
                buffer[i]->reset();
            }
        }

        void init(int _bufferSize){
            bufferSize = _bufferSize;
            buffer = new LogicGate*[_bufferSize];
            for (int i=0; i<_bufferSize; i++){
                buffer[i] = nullptr;
            }
        }

        LogicGate& add(eLogicType logicType, int inputSize=0){
            if (editable){
                LogicGate* g = new LogicGate(logicType);
                g->master = this;
                g->refNumber = at;
                g->setInputSize(inputSize);
                buffer[at++] = g;
                return *g;
            }else{
                print("Circuit is a component circuit and is not editable.");
                throw ERROR_NOT_EDITABLE;
            }
        }

        Circuit& add(Circuit & c){
            if(editable){
                Circuit* _c = (Circuit*)c.copy();
                _c->master = this;
                _c->refNumber = at;
                buffer[at++] = _c;
                return *_c;
            }
            else{
                print("Circuit is a component circuit and is not editable.");
                throw ERROR_NOT_EDITABLE;
            }
        }

        bool transform(){
            for (int i=0; i < at; i++){
                if (!buffer[i]->executed){
                    buffer[i]->exec();
                }
            }
            if (outputs.size()>0){
                return outputs[0]->_output;
            }else{
                return false;
            }
        }

        bool exec(){
            if (!executed){
                //print"#0003");
                //print":)");
                executed = true;
                int sz = inputs.size();
                for (int i=0; i<sz; i++){
                    inputs[i]->exec();
                }
                _output = transform();
            }else{
                //print"!0003");
            }
            return _output;
        }

        bool run(){
            //print"#0001");
            reset();
            //print"#0002");
            exec();
            return true;
        }

        int addAsInput(LogicGate* g, std::string _name){
            if (editable){
                int sz = inputs.size();
                inputs.push_back(g);
                inputNames.push_back(_name);
                return sz;
            }else{
                return -1;
            }
        }

        int addAsOutput(LogicGate* g, std::string _name){
            if (editable){
                int sz = outputs.size();
                outputs.push_back(g);
                outputNames.push_back(_name);
                return sz;
            }
            else{
                return -1;
            }
        }

        LogicGate& input(std::string _name){
            int i;
            int sz = inputNames.size();
            for (i=0; i<sz; i++){
                if (_name == inputNames[i]){
                    return *inputs[i];
                }
            }
            print("Trying to get unexisting key.");
            throw ERROR_UNEXISTING_KEY;
        }

        LogicGate& output(std::string _name){
            int i;
            int sz = outputNames.size();
            for (i=0; i<sz; i++){
                if (_name == outputNames[i]){
                    return *outputs[i];
                }
            }
            print("Trying to get unexisting key.");
            throw ERROR_UNEXISTING_KEY;
        }

        void display(){
            print("Inputs:");
            for (int i=0; i<inputs.size(); i++){
                print(inputNames[i] << ": " << inputs[i]->_output);
            }

            print("\nOutputs:");
            for (int i=0; i<outputs.size(); i++){
                print(outputNames[i] << ": " << outputs[i]->_output);
            }
        }
};

void LogicGate::exportInput(std::string _name){
    isIO = true;
    ioType = IO_INPUT;
    Circuit* c = (Circuit*) master;
    refPin = -c->addAsInput(this, _name);
}

void LogicGate::exportOutput(std::string _name){
    isIO = true;
    ioType = IO_OUTPUT;
    Circuit* c = (Circuit*) master;
    refPin = c->addAsOutput(this, _name);
}

bool LogicGate::exec()
{
    if (!executed){
        executed = true;
        if (ioType == IO_OUTPUT){
            Circuit* c = (Circuit*)master;
            c->exec();
        }
        for (int i=0; i<_inputSize; i++){
            if (_input[i]){
                _input[i]->exec();
            }else{
                break;
            }
        }
        _output = transform();
    }
    return _output;
}


Circuit* workingCircuit;

int main(){
    // The following is an full adder example:
    circuit_init(half_adder, 100);
    def(a, SWITCH, 1);
    def(b, SWITCH, 1);
    def(xor_up, XOR, 2);
    def(and_down, AND, 2);
    def(_sum, BUFFER, 1);
    def(_carry, BUFFER, 1);
    a >> xor_up; a >> and_down;
    b >> xor_up; b >> and_down;
    xor_up >> _sum;
    and_down >> _carry;
    a.exportInput("a");
    b.exportInput("b");
    _sum.exportOutput("sum");
    _carry.exportOutput("carry");
    a.toggle();
    b.toggle();
    
    circuit_init(full_adder, 100);
    def(_a, SWITCH, 1);
    def(_b, SWITCH, 1);
    def(_cIn, SWITCH, 1);
    def(_sum2, BUFFER, 1);
    def(_cOut, BUFFER, 1);
    Circuit& ha_up = full_adder.add(half_adder);
    Circuit& ha_down = full_adder.add(half_adder);
    _a >> ha_up.input("a");
    _b >> ha_up.input("b");
    ha_up.output("sum") >> ha_down.input("a");
    _cIn >> ha_down.input("b");
    def(or_down, OR, 2);
    ha_up.output("carry") >> or_down;
    ha_down.output("carry") >> or_down;
    ha_down.output("sum") >> _sum2;
    or_down >> _cOut;
    _a.exportInput("a");
    _b.exportInput("b");
    _cIn.exportInput("cin");
    _sum2.exportOutput("sum");
    _cOut.exportOutput("cout");

    Circuit& fa = full_adder.make();
    set_circuit(fa);

    print("a0b0c0");
    cexec;
    fa.display();

    print("a1b0c0");
    fa.input("a").toggle();
    cexec;
    fa.display();

    print("a0b1c0");
    fa.input("a").toggle();
    fa.input("b").toggle();
    cexec;
    fa.display();

    print("a1b1c0");
    fa.input("a").toggle();
    cexec;
    fa.display();

    print("a0b0c1");
    fa.input("a").setValue(0);
    fa.input("b").setValue(0);
    fa.input("cin").setValue(1);
    cexec;
    fa.display();

    print("a1b0c1");
    fa.input("a").setValue(1);
    fa.input("b").setValue(0);
    fa.input("cin").setValue(1);
    cexec;
    fa.display();

    print("a0b1c1");
    fa.input("a").setValue(0);
    fa.input("b").setValue(1);
    fa.input("cin").setValue(1);
    cexec;
    fa.display();

    print("a1b1c1");
    fa.input("a").setValue(1);
    fa.input("b").setValue(1);
    fa.input("cin").setValue(1);
    cexec;
    fa.display();
    return 0;
}

/*
int main(){
    std::cout << "Start\n";
    circuit_init(_c,10000);
    def(bfr, BUFFER, 1);
    def(nor_u, NOR, 2);
    def(nor_d, NOR, 2);
    def(and_u, AND, 2);
    def(and_d, AND, 2);
    def(notbfr, NOT, 1);
    def(set, SWITCH, 0);
    def(setBit, SWITCH, 0);
    set.exportInput("set");
    setBit.exportInput("bit");
    bfr.exportOutput("out");
    set >> and_u;
    setBit >> and_d;
    set >> and_d;
    setBit >> notbfr;
    notbfr >> and_u;
    and_u >> nor_u;
    and_d >> nor_d;
    nor_u >> nor_d;
    nor_d >> nor_u;
    nor_u >> bfr;
    //print"Making _c:");
    Circuit c2 = _c.make();
    //print"Made");
    set_circuit(c2);

    print("Set Bit 0, Set 0 =>");
    cexec;

    print("Set Bit 1, Set 0 =>");
    c2.input("bit").toggle();
    // c2.input("bit").display();
    cexec;

    print("Set Bit 1, Set 1 =>");
    c2.input("set").toggle();
    cexec;

    print("Set Bit 0, Set 0 =>");
    c2.input("set").toggle();
    c2.input("bit").toggle();
    cexec;

    print("Set Bit 0, Set 1 =>");
    c2.input("set").toggle();
    cexec;

    print("Set Bit 0, Set 0 =>");
    c2.input("set").toggle();
    cexec;
    return 0;
}
*/