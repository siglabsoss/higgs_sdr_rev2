from menu import *


main = Menu(title = "Main Menu")
sub = Menu(title = "Submenu")
main.set_options([
("Open submenu", sub.open),
("Close main menu", main.close)
])
sub.set_options([
("Return to main menu", sub.close)
])
main.open()
