_Advanced game Tweak topics: Surfaces and Tires. At end a Links section for general info._  

## Game (advanced)
   
### Surfaces

Parameters are in surfaces.cfg.  
_ToDo: more info_

![](images/12-surfaces.jpg)

   
### Tires

**Tire editing** is possible in game during drive.  
This is a specific Graphs mode. On left showing resulting Tire Forces graphs (for 4 load forces).  
Graphs show both: longitudinal (Fx) force in yellow and lateral (Fy) in cyan colors.  

On right is the editor (list) with one set of parameters shown longitudinal or lateral (Home key toggle).  
This editor is a simple list of: parameter name, value and description.  
Using keys to move cursor `<` (PgUp, PgDn) and change selected value (Num /, Num *).  

There is more info on Tweak tab about keys and sliders for scale of Tire graphs scale, etc.  
Tires can be saved and loaded using Game Editor window (Alt-Z) on Tires tab.  

![](images/10-tire-edit.jpg)

Left bottom also visible on screen are **Tire circles**. This visualization can be toggled.  
It shows each tire force in green (orange when exceeded in combine method),  
grip budget (force circle) an yellow circle (actually an ellipse), which size depends on wheel load force.  

Next screen shows all 4 wheels with dynamic tire graphs (Fx, Fy like before),  
but for actual load (and camber) given for each wheel.

![](images/11-tire4.jpg)

#### Notes

Latest VDrift also has tire forces combining parameters (other set), and more tire models.  
But SR is based on VDrift sources from 2010, which did not have them yet.  
A lot was modified in SR and in VDrift, thus difficult to update.  

Tire editing is meant only for people with good knowledge of Pacejka's Magic Formula.  
And for those who want to tinker with it.

It can be very time consuming/wasting, and produce no better results.  
_Automatic testing for car handling would be better (like Perf test)._  

