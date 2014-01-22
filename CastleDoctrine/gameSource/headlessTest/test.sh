
for i in {1..10}
do
    cat testRequest.txt | telnet localhost 5077
done

cat testQuit.txt | telnet localhost 5077