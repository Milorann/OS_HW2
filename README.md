### Листеренко Ольга Руслановна ###
### БПИ 217 ###
  
### Вариант 1 ###
### Задача ###
**Задача о парикмахере**. В тихом городке есть парикмахерская. Салон парикмахерской мал, работать в нем нем может только **один парикмахер**, обслуживающий **одного посетителя**. Есть несколько кресел для ожидания в очереди. Парикмахер всю жизнь обслуживает посетителей. Когда в салоне никого нет, он спит в кресле. Когда посетитель приходит и видит спящего парикмахера, он будит его, садится в кресло, «засыпая» на тот момент, пока парикмахер обслуживает его. Если посетитель приходит, а парикмахер занят, то он встает в очередь и «засыпает». После стрижки парикмахер сам провожает посетителя. Если есть ожидающие посетители, то парикмахер будит одного из них и ждет пока тот сядет в кресло парикмахера и начинает стрижку. Если никого нет, он снова садится в свое кресло и засыпает до прихода посетителя. **Создать приложение, моделирующее рабочий день парикмахерской**. *Парикмахера и каждого из посетителей моделировать в виде отдельных процессов. Последние могут независимо порождаться в произвольное время и завершать работу после обслуживания*.  

### Сценарий на 4, 5, 6 ###
В начальном процессе происходит форк.  
Родительский процесс выполняет функции парикмахера. Парикмахер увеличивает семафор кресла, делая его доступным для посетителей, и спит с помощью другого семафора, который клиент увеличивает в своем процессе, до того, как прийдет клиент. Ребенок занимается созданием процессов-клиентов. Клиенты появляются случайным образом в диапазоне от 1 до 10 секунд. Каждый процесс процесс пытается занять кресло. Если оно занято, то засыпает до того момента, как парикмахер увеличит семафор кресла. До того, как пробудить парикмахера, клиент в разделяемую память записывает свой pid, а затем пробуждает парикмахера. Парикмахер читает pid и начинает стрижку. Стрижка длится 5 секунд, после чего клиент уходит (процесс убивается по pid), а семафор кресла увеличивается и парикмахер засыпает. 


### 4 балла ###
**Разработать консольное приложение, в котором единый родительский процесс запускает требуемое число дочерних процессов**.  
Множество процессов взаимодействуют с использованием неименованных POSIX семафоров расположенных в разделяемой памяти. Обмен данными также ведется через разделяемую память в стандарте POSIX.  
Завершение программы осуществляется по прерыванию с клавиатуры. В задании другие способы завершения не описаны. При завершении программы все семафоры и разделяемая память закрываются.
