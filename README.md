# qTimeLineEditor

![Image1](/pics/qTimeLineEditor.jpg)

It's a graphical time line editor to help you create and prototype animations. It is useful for adjusting variables and checking out how the effects changes over time with keyframing and easing/twining functions. It may also have some similarities with the timeline component of adobe flash, after effects, edge animate or other animation software.


It's originally created to import visual Fxs on media objects, of a real time presentational system. For each media object, you can add/delete/move/size starting and ending fxs, and add/delete/move/size subFxs (blue parts that can avoid overlapping each other by jumping on them) inside this media object. You can also grab the play head to move the time along the editor with scrolling prediction at the edges. Currently you can not add more tracks usually stacked vertically, allowing for multiple pieces of media objects, since I didn’t implement this feature as this project was abandoned years ago. This it wouldn’t be a serious problem to solve, since each media object is a single class inside the source code. Timeline has time code markers indicating the location of each frame. Every time you gab the play head a small yellow tip window on the top is appearing, to indicate the exact time you want to move.  This also happens when you move the media object but the tip window label is green in this case and includes also the fx duration in seconds. When you right click on media object, a popup window appear that includes options like duration, delete, move to and add New Fx. Some of them are also not implemented yet. 


As I already said, this library it is built primary for internal custom projects, but its abandoned, so you may find bugs and non-working parts. Anyway, feel free to send me suggestions or requests.


You can run directly the executable file in the release folder to give it a try. Last but not least, I want to thank Jeffrey M. Barber with his “ztimeline” library which gave rise to the development of this code.

Enjoy.


![Image2](/pics/qTimeLineEditor1.jpg)
![Image3](/pics/qTimeLineEditor2.jpg)
![Image4](/pics/qTimeLineEditor3.jpg)

