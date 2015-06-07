now=$(date +"%T")
echo "Current time : $now"
python test_captain_thread.py 10 10 10 100
now=$(date +"%T")
echo "Current time : $now"
echo "test_captain_thread 10 10 10 10 DONE"

now=$(date +"%T")
echo "Current time : $now"
python test_captain_random_thread.py 
now=$(date +"%T")
echo "Current time : $now"
echo "test_captain_random_thread 10 10 100 DONE"

