func : (cond : bool, cond2 : bool) int {
a : int;
a = 4;
a++;
give "string";
if(!true){
b : int;
b--;
}
if(cond){
t: int;
t = 5;
}
else{
t: bool;
t = false;
}
while(cond2){
cond2 = false;
}
return a;
}
main : () int {
func(true, false);
}