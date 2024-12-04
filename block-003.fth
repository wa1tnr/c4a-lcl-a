block #003 - strings
find fill loaded?
p1vhere $100 + ;   p2p1 $100 + ;
fill(to num char--)swap ?dup
   if>t swap t> for over over c! 1+ nextthen2drop ;
fill-c(to num char--)swap ?dup
   if>t swap t> for over over ! cell+ next 2dropthen;
cmove(src dst n--)>r >t >a
   r> ?dupiffor @a+ !t+ nextthenatdrop ;
cmove>(src dst n--)>r r@ + >t  r@ + >a
   r> ?dupif1+ for @a- !t- nextthenatdrop ;





