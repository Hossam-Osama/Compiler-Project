// nfa_generator.cpp
// Build NFA from lexical rules (Part 1 of the assignment).
// Compile: g++ -std=c++17 -O2 nfa_generator.cpp -o nfa_generator
// Usage: ./nfa_generator rules.txt

#include <bits/stdc++.h>
using namespace std;

/*
Design choices & simplifications:
- Token labels on transitions are strings (e.g., "a", "b", "a-z", "0-9", "if", etc.).
  We treat them as atomic matchers for NFA construction printing/visualization.
- We support:
    * Alternation: |
    * Kleene star: *
    * One-or-more: +   (implemented as r r*)
    * Concatenation: implicit (we insert explicit '.' operator in the parser)
    * Grouping: ( )
    * Lambda: \L (an epsilon marker used in definitions)
    * Escapes: backslash escapes next char (e.g., \= or \+ or \.)
- Definitions using '=' are textual macros and will be expanded before parsing expressions.
- Keywords: { a b c } are converted to alternation of literal tokens.
- Punctuations: [ ; , \( \) { } ] are handled similarly.
*/

struct State {
    int id;
    vector<pair<string,int>> trans; // labeled transitions (label, target state)
    vector<int> eps;                // epsilon transitions (empty label)
};

struct NFA {
    int start;
    int accept;
    vector<State> states;
    string tokenName; // which token this NFA represents (empty for combined)
};

int new_state_id_global = 0;
int fresh_state_id() { return new_state_id_global++; }

string trim(const string &s){
    size_t a=0,b=s.size();
    while(a<b && isspace((unsigned char)s[a])) ++a;
    while(b>a && isspace((unsigned char)s[b-1])) --b;
    return s.substr(a,b-a);
}

// ------------------- Input parsing helpers -------------------

vector<string> split_top_level(const string &s, char delim) {
    // split s by delim but not inside parentheses
    vector<string> out;
    int level = 0;
    string cur;
    for (size_t i=0;i<s.size();++i){
        char c=s[i];
        if(c=='(') level++;
        if(c==')') level--;
        if(c==delim && level==0){
            out.push_back(cur);
            cur.clear();
        } else cur.push_back(c);
    }
    if(!cur.empty()) out.push_back(cur);
    for(auto &x : out) x = trim(x);
    return out;
}

string unescape(const string &s){
    string r;
    for(size_t i=0;i<s.size();++i){
        if(s[i]=='\\' && i+1<s.size()){
            r.push_back(s[i+1]);
            ++i;
        } else r.push_back(s[i]);
    }
    return r;
}

// Take something like "a-z" and produce "a-z" label — kept as-is
// We'll treat range tokens as single atomic labels "a-z"
bool looks_like_range(const string &s){
    // crude: contains '-' and length >=3 like a-z or 0-9
    if(s.size()>=3 && s.find('-')!=string::npos) return true;
    return false;
}

// Tokenize RHS into basic atomic tokens (literals, ranges, identifiers)
vector<string> tokenize_rhs(const string &rhs){
    vector<string> tokens;
    for(size_t i=0;i<rhs.size();){
        char c = rhs[i];
        if(isspace((unsigned char)c)){ ++i; continue; }
        if(c=='|' || c=='(' || c==')' || c=='*' || c=='+' ){
            tokens.push_back(string(1,c));
            ++i; continue;
        }
        if(c=='\\'){ // escape
            if(i+1<rhs.size()){
                tokens.push_back("\\" + string(1, rhs[i+1]));
                i+=2; continue;
            } else { tokens.push_back("\\"); ++i; continue; }
        }
        // collect contiguous "word" (letters digits - . etc)
        string w;
        while(i<rhs.size() && !isspace((unsigned char)rhs[i]) && string("|()*+").find(rhs[i])==string::npos){
            w.push_back(rhs[i]);
            ++i;
        }
        tokens.push_back(w);
    }
    return tokens;
}

// ------------------- Convert infix regex to postfix (shunting-yard) -------------------
// Operators: * (unary, highest), + (unary), concatenation '.' (binary), '|' (binary)
int prec(const string &op){
    if(op=="*" || op=="+") return 4;
    if(op==".") return 3;
    if(op=="|") return 1;
    return 0;
}
bool is_op(const string &s){
    return s=="*"||s=="+"||s=="."||s=="|";
}

vector<string> insert_concat(const vector<string> &tokens){
    // insert '.' where concatenation is implied
    vector<string> out;
    for(size_t i=0;i<tokens.size();++i){
        string t = tokens[i];
        out.push_back(t);
        if(i+1<tokens.size()){
            string a = t, b = tokens[i+1];
            bool a_is_sym = (a==")" || a=="*" || a=="+" );
            bool b_is_sym = (b=="(");
            bool a_is_token = !is_op(a) && a!="(" && a!=")";
            bool b_is_token = !is_op(b) && b!="(" && b!=")";
            // Cases where concat is needed:
            if( (a_is_token || a==")" || a=="*" || a=="+") &&
                (b_is_token || b=="(") ){
                out.push_back(".");
            }
        }
    }
    return out;
}

vector<string> to_postfix(const vector<string> &tokens_in){
    vector<string> tokens = insert_concat(tokens_in);
    vector<string> output;
    vector<string> stack;
    for(auto &t : tokens){
        if(t=="("){
            stack.push_back(t);
        } else if(t==")"){
            while(!stack.empty() && stack.back()!="("){
                output.push_back(stack.back()); stack.pop_back();
            }
            if(!stack.empty() && stack.back()=="(") stack.pop_back();
        } else if(is_op(t)){
            while(!stack.empty() && is_op(stack.back()) &&
                  ( (prec(stack.back()) > prec(t)) ||
                   (prec(stack.back())==prec(t) && t!="*") ) ){
                output.push_back(stack.back()); stack.pop_back();
            }
            stack.push_back(t);
        } else {
            // operand
            output.push_back(t);
        }
    }
    while(!stack.empty()){ output.push_back(stack.back()); stack.pop_back(); }
    return output;
}

// ------------------- Thompson Construction -------------------

NFA make_basic(const string &label){
    NFA n;
    n.start = fresh_state_id();
    n.accept = fresh_state_id();
    n.states.resize( max(n.start,n.accept) + 1 );
    for(auto &s : n.states){ /* nothing */ }
    // ensure correct ids
    n.states[0].id = 0; // placeholder; will reset below
    // We'll ensure states vector indexes are ids by resizing at combine time
    // For now, create two states with given ids
    n.states.clear();
    State s1; s1.id = n.start;
    State s2; s2.id = n.accept;
    s1.trans.push_back({label, n.accept});
    n.states.push_back(s1); // index 0
    n.states.push_back(s2); // index 1
    n.tokenName = "";
    return n;
}

// Helper to re-index NFA states so that their ids are contiguous and correct
void renumber_nfa(NFA &n, int base){
    // current states vector contains states with .id fields properly set (we used new ids)
    // We will not change ids here, caller must ensure base aligns with IDs usage.
    (void)n; (void)base;
}

NFA nfa_concat(const NFA &a, const NFA &b){
    NFA out;
    // shift b's states ids to ensure uniqueness? In our construction we already used global fresh ids
    // so ids are unique. We'll merge states vector merging.
    // start is a.start, accept is b.accept
    out.start = a.start;
    out.accept = b.accept;
    // merge states: gather from a and b
    unordered_map<int, State> mapStates;
    for(auto &s : a.states) mapStates[s.id] = s;
    for(auto &s : b.states) mapStates[s.id] = s;
    // add epsilon from a.accept to b.start
    mapStates[a.accept].eps.push_back(b.start);
    // build vector sorted by id
    vector<int> ids;
    for(auto &kv : mapStates) ids.push_back(kv.first);
    sort(ids.begin(), ids.end());
    out.states.clear();
    for(int id : ids) out.states.push_back(mapStates[id]);
    out.tokenName = "";
    return out;
}

NFA nfa_alternate(const NFA &a, const NFA &b){
    NFA out;
    int s = fresh_state_id();
    int f = fresh_state_id();
    out.start = s;
    out.accept = f;
    unordered_map<int, State> mapStates;
    for(auto &st : a.states) mapStates[st.id] = st;
    for(auto &st : b.states) mapStates[st.id] = st;
    // ensure a.accept and b.accept exist
    // new start with eps to a.start and b.start
    State S; S.id = s; S.eps.push_back(a.start); S.eps.push_back(b.start);
    State F; F.id = f;
    // add eps from a.accept and b.accept to new accept
    mapStates[a.accept].eps.push_back(f);
    mapStates[b.accept].eps.push_back(f);
    mapStates[s] = S;
    mapStates[f] = F;
    vector<int> ids;
    for(auto &kv : mapStates) ids.push_back(kv.first);
    sort(ids.begin(), ids.end());
    out.states.clear();
    for(int id: ids) out.states.push_back(mapStates[id]);
    out.tokenName = "";
    return out;
}

NFA nfa_kleene(const NFA &a){
    NFA out;
    int s = fresh_state_id();
    int f = fresh_state_id();
    out.start = s;
    out.accept = f;
    unordered_map<int, State> mapStates;
    for(auto &st : a.states) mapStates[st.id] = st;
    State S; S.id = s; S.eps.push_back(a.start); S.eps.push_back(f);
    State F; F.id = f;
    mapStates[a.accept].eps.push_back(a.start);
    mapStates[a.accept].eps.push_back(f);
    mapStates[s] = S;
    mapStates[f] = F;
    vector<int> ids;
    for(auto &kv : mapStates) ids.push_back(kv.first);
    sort(ids.begin(), ids.end());
    out.states.clear();
    for(int id: ids) out.states.push_back(mapStates[id]);
    out.tokenName = "";
    return out;
}

// + (one or more) : r+ = r r*
NFA nfa_plus(const NFA &a){
    NFA aa = a;
    NFA a_star = nfa_kleene(a);
    NFA res = nfa_concat(aa, a_star);
    return res;
}

// Build NFA from postfix token list (operands are atomic labels)
NFA build_from_postfix(const vector<string> &postfix){
    vector<NFA> st;
    for(auto &tk : postfix){
        if(tk=="."){
            if(st.size()<2){ cerr<<"concat needs 2 operands\n"; break; }
            NFA b = st.back(); st.pop_back();
            NFA a = st.back(); st.pop_back();
            NFA c = nfa_concat(a,b);
            st.push_back(c);
        } else if(tk=="|"){
            if(st.size()<2){ cerr<<"alt needs 2 operands\n"; break; }
            NFA b = st.back(); st.pop_back();
            NFA a = st.back(); st.pop_back();
            NFA c = nfa_alternate(a,b);
            st.push_back(c);
        } else if(tk=="*"){
            if(st.empty()){ cerr<<"star needs 1 operand\n"; break; }
            NFA a = st.back(); st.pop_back();
            st.push_back(nfa_kleene(a));
        } else if(tk=="+"){
            if(st.empty()){ cerr<<"plus needs 1 operand\n"; break; }
            NFA a = st.back(); st.pop_back();
            st.push_back(nfa_plus(a));
        } else {
            // operand: token label (we handle \L specially)
            string label = tk;
            if(label=="\\L"){ // epsilon
                // Create an NFA that is epsilon from start to accept
                NFA n;
                n.start = fresh_state_id();
                n.accept = fresh_state_id();
                State s; s.id = n.start; s.eps.push_back(n.accept);
                State f; f.id = n.accept;
                n.states.push_back(s); n.states.push_back(f);
                st.push_back(n);
            } else {
                // basic symbol / range / literal
                string lab = label;
                // if it's an escaped single, remove the backslash for readability
                if(lab.size()>=2 && lab[0]=='\\') lab = lab.substr(1);
                NFA basic = make_basic(lab);
                st.push_back(basic);
            }
        }
    }
    if(st.empty()){
        // empty expression -> epsilon
        NFA n;
        n.start = fresh_state_id();
        n.accept = fresh_state_id();
        State s; s.id = n.start; s.eps.push_back(n.accept);
        State f; f.id = n.accept;
        n.states.push_back(s); n.states.push_back(f);
        return n;
    }
    return st.back();
}

// ------------------- Utilities to merge many NFAs into one combined NFA -------------------

NFA combine_all(const vector<pair<NFA,string>> &nfas){
    // new start state
    NFA out;
    int s0 = fresh_state_id();
    int maxId = s0;
    unordered_map<int, State> mapStates;
    for(auto &p : nfas){
        const NFA &n = p.first;
        // insert states
        for(auto &st : n.states){
            mapStates[st.id] = st;
            if(st.id > maxId) maxId = st.id;
        }
    }
    // new start
    State S0; S0.id = s0;
    for(auto &p : nfas){
        S0.eps.push_back(p.first.start);
    }
    mapStates[s0] = S0;
    // build out.states sorted
    vector<int> ids;
    for(auto &kv : mapStates) ids.push_back(kv.first);
    sort(ids.begin(), ids.end());
    out.states.clear();
    for(int id : ids) out.states.push_back(mapStates[id]);
    out.start = s0;
    // create a fake combined accept? We will not add a single accept, each original accept will remain labeled
    out.accept = -1;
    out.tokenName = "";
    return out;
}

// ------------------- Printing / Dot output -------------------

void print_nfa(const NFA &n, const unordered_map<int,string> &acceptLabels){
    cout << "Combined NFA start state: " << n.start << "\n";
    for(auto &st : n.states){
        cout << "State " << st.id;
        if(acceptLabels.find(st.id)!=acceptLabels.end()){
            cout << " [ACCEPT for token: " << acceptLabels.at(st.id) << "]";
        }
        cout << "\n";
        for(auto &e : st.eps){
            cout << "  -- eps --> " << e << "\n";
        }
        for(auto &t : st.trans){
            cout << "  -- " << t.first << " --> " << t.second << "\n";
        }
    }
}

void write_dot(const NFA &n, const unordered_map<int,string> &acceptLabels, const string &filename){
    ofstream f(filename);
    f << "digraph NFA {\n";
    f << "  rankdir=LR;\n";
    // nodes
    for(auto &st : n.states){
        string label = to_string(st.id);
        if(acceptLabels.find(st.id)!=acceptLabels.end()){
            f << "  " << st.id << " [shape=doublecircle,label=\"" << st.id << "\\n(" << acceptLabels.at(st.id) << ")\"];\n";
        } else {
            f << "  " << st.id << " [shape=circle,label=\"" << st.id << "\"];\n";
        }
    }
    // start arrow
    f << "  start [shape=point];\n";
    f << "  start -> " << n.start << ";\n";
    // edges
    for(auto &st : n.states){
        for(auto &e : st.eps){
            f << "  " << st.id << " -> " << e << " [label=\"ε\"];\n";
        }
        for(auto &t : st.trans){
            string lab = t.first;
            // escape quotes
            for(char &c : lab) if(c=='\"') c='\'';
            f << "  " << st.id << " -> " << t.second << " [label=\"" << lab << "\"];\n";
        }
    }
    f << "}\n";
    f.close();
}

// ------------------- Main driver: parse rules, build NFAs -------------------

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(argc<2){
        cerr<<"Usage: "<<argv[0]<<" <rules_file>\n";
        return 1;
    }
    string rules_path = argv[1];
    ifstream fin(rules_path);
    if(!fin){ cerr<<"Cannot open rules file: "<<rules_path<<"\n"; return 1; }
    vector<string> lines;
    string line;
    while(getline(fin,line)){
        // remove comments (if any) and trim
        string t = line;
        // ignore windows BOM etc (rare)
        if(!t.empty() && (unsigned char)t[0]==0xEF) t = t.substr(1);
        t = trim(t);
        if(t.empty()) continue;
        lines.push_back(t);
    }
    fin.close();

    // store definitions and expressions
    unordered_map<string,string> definitions;
    vector<pair<string,string>> expressions; // (tokenName, RHS)
    vector<string> keywords_lines;
    vector<string> punct_lines;

    for(auto &ln : lines){
        // identify patterns
        if(ln.find('=')!=string::npos && ln.find(':')==string::npos){
            // definition
            size_t pos = ln.find('=');
            string L = trim(ln.substr(0,pos));
            string R = trim(ln.substr(pos+1));
            definitions[L] = R;
        } else if(ln.find(':')!=string::npos){
            size_t pos = ln.find(':');
            string L = trim(ln.substr(0,pos));
            string R = trim(ln.substr(pos+1));
            expressions.push_back({L,R});
        } else if(!ln.empty() && ln.front()=='{' && ln.back()=='}'){
            keywords_lines.push_back(ln);
        } else if(!ln.empty() && ln.front()=='[' && ln.back()==']'){
            punct_lines.push_back(ln);
        } else {
            // unknown line; try to accept keywords/punct if they have braces spaced
            if(ln.front()=='{' && ln.back()=='}') keywords_lines.push_back(ln);
            else if(ln.front()=='[' && ln.back()==']') punct_lines.push_back(ln);
            else {
                // ignore
                //cerr<<"Ignoring line: "<<ln<<"\n";
            }
        }
    }

    // process keywords lines into expressions (as separate token specs)
    for(auto &kwln : keywords_lines){
        string inner = kwln.substr(1, kwln.size()-2);
        vector<string> kws = split_top_level(inner, ' ');
        for(auto &kw : kws){
            if(kw.empty()) continue;
            string tokenName = "KEYWORD_" + kw;
            // treat keyword as literal sequence e.g. "if" => i f (concatenation)
            // We'll represent as a single operand literal "if" for Thompson (works as atomic label)
            expressions.push_back({tokenName, kw});
        }
    }
    // process punct lines similarly; entries separated by spaces or commas
    for(auto &pl : punct_lines){
        string inner = pl.substr(1, pl.size()-2);
        vector<string> parts = split_top_level(inner, ' ');
        for(auto &pt : parts){
            if(pt.empty()) continue;
            string t = pt;
            // some punctuation may be like '\(' or '\)' or ';'
            // token name:
            string tokenName = "PUNCT_" + t;
            expressions.push_back({tokenName, t});
        }
    }

    // Expand definitions inside RHS by simple textual substitution of definition names
    // Caution: we only replace whole tokens (space-separated) to avoid accidental partial replacements.
    // We'll iterate until no changes (to allow nested defs).
    auto expand_once = [&](const string &rhs)->string{
        vector<string> toks = tokenize_rhs(rhs);
        for(size_t i=0;i<toks.size();++i){
            string tk = toks[i];
            // if token matches a definition name exactly, replace with parentheses of the def content
            if(definitions.find(tk)!=definitions.end()){
                string repl = "(" + definitions[tk] + ")";
                toks[i] = repl;
            }
        }
        string out;
        for(size_t i=0;i<toks.size();++i){
            if(i) out += " ";
            out += toks[i];
        }
        return out;
    };

    // Expand definitions recursively for expressions
    vector<pair<string,string>> expanded_exprs;
    for(auto &p : expressions){
        string name = p.first;
        string rhs = p.second;
        string prev;
        int guard=0;
        while(rhs!=prev && guard<20){
            prev = rhs;
            rhs = expand_once(rhs);
            guard++;
        }
        rhs = trim(rhs);
        expanded_exprs.push_back({name, rhs});
    }

    // For definitions themselves, we may want to pre-expand their rhs as well (optional)
    // Now build NFA for each expression
    vector<pair<NFA,string>> built;
    unordered_map<int,string> acceptLabels; // map accept-state -> token name

    for(auto &p : expanded_exprs){
        string tokenName = p.first;
        string rhs = p.second;
        // Tokenize RHS into tokens for regex processing
        vector<string> tokens = tokenize_rhs(rhs);
        // Convert tokens -> postfix
        vector<string> postfix = to_postfix(tokens);

        // Build NFA from postfix
        NFA n = build_from_postfix(postfix);
        // mark accept state label
        acceptLabels[n.accept] = tokenName;
        n.tokenName = tokenName;
        // store
        built.push_back({n, tokenName});
    }

    // Combine all NFAs into one big NFA with a new start state
    NFA combined = combine_all(built);

    // Merge accept labels map for combined NFA (we have acceptLabels already keyed by old accept state ids)
    unordered_map<int,string> acceptLabelsCombined = acceptLabels;

    // Print result
    cout << "=== Phase 1: NFA Generator ===\n";
    cout << "Input rules file: " << rules_path << "\n";
    cout << "Number of token NFA built: " << built.size() << "\n\n";

    print_nfa(combined, acceptLabelsCombined);

    // Write DOT file
    string dotfile = "combined_nfa.dot";
    write_dot(combined, acceptLabelsCombined, dotfile);
    cout << "\nDOT file written: " << dotfile << " (use 'dot -Tpng combined_nfa.dot -o nfa.png')\n";

    // Also output a debug listing of each token's start->accept
    cout << "\nToken start/accept pairs:\n";
    for(auto &p : built){
        cout << "Token: " << p.second << "  start=" << p.first.start << " accept=" << p.first.accept << "\n";
    }

    cout << "\nDone.\n";

    return 0;
}
