
from kivy.app import App
from kivy.uix.label import Label
import bronker
import _thread
#
class BronkerGUI (App):
    
    def build(self):
        #criando widgets
        _thread.start_new_thread(bronker.run, (False,))
        return Label(text="Bronker Launcher")
#
if __name__ == "__main__":
    BronkerGUI().run()
#
