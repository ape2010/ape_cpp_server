namespace cpp teststudent

struct Student{
	1: string no,
	2: string name,
	3: i16 age,
}
service Serv{
	Student QueryByNo(1: string no),
}
