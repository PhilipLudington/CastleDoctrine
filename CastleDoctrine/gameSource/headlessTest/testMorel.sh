
for i in {1..5}
do
    cat testRequestMorel.txt | telnet localhost 5077
done

cat testQuit.txt | telnet localhost 5077