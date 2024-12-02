( Block 3 - Strings )
find fill loaded?
p1vhere $100 + ;   p2p1 $100 + ;
fill  (dst cnt ch--) swap ?dupif>t swap t> for over over c!
fill-c(dst cnt n--)  swap ?dupif>t swap t> for over over !
cmove(src dst n--)>r >t >a
   r> ?dupiffor @a+ !t+ nextthen
   atdrop ;
cmove>(src dst n--)>r  r@ + >t  r@ + >a
   r> ?dupif1+ for @a- !t- nextthen
   atdrop ;




                                                               
