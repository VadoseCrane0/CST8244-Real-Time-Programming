Title 
{PartB_sigproc}
Status
{
I dont believe any requirements are missing. status is "Working as expected"
}
Known Issues{
if you terminate from the IDE it will create zombies. I also didnt handle SIGUSR2 so if that is called on the parent it 
will kill the parent. I dont think it was a requirement to handle these ways of dying though. Forgive me if i was wrong.
}

Expected Grade
{A}