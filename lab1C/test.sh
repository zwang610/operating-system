touch f.txt
echo 0 > f0.txt
echo 1 > f1.txt
echo 2 > f2.txt
touch out.txt
touch err.txt

echo "--TEST 1"
./simpsh --wronly f0.txt --wronly f1.txt --rdonly f2.txt \
  --command 2 0 1 ivld --verbose --rdonly f.txt --wait >out.txt 2>err.txt
if [ $? -eq 1 ] && [ -s out.txt ]
then 
    echo "test1 passed" 
else 
    echo "test1 failed"
fi

rm *txt

touch f.txt
echo 0 > f0.txt
echo 1 > f1.txt
echo 2 > f2.txt
touch out.txt
touch err.txt

echo "--TEST 2"
./simpsh --wronly f0.txt --wronly f1.txt --rdonly f2.txt --command 2 0 1 cat >out.txt 2>err.txt
if [ $? -eq 0 ] && [ ! -s out.txt ]
then 
    echo "test2 passed" 
else 
    echo "test2 failed"
fi

rm *txt
touch f.txt
echo 0 > f0.txt
echo 1 > f1.txt
echo 2 > f2.txt
touch out.txt
touch err.txt

echo "--TEST 3"
./simpsh --abort >out.txt err>txt
if [ $? -eq 139 ] 
then 
    echo "test3 passed" 
else 
    echo "test3 failed"
fi

rm *txt

echo "DONE ..."
cd ..

