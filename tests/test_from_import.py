from mymod import greet, ANSWER as THE_ANSWER
import mymod as mm

greet("from import test")
print("THE_ANSWER =", THE_ANSWER)
mm.greet("alias test")
