const fs = require('fs');

let data = fs.readFileSync('dataset.txt');
let json = JSON.parse(data);

json.forEach(element => {
    ranges = element["data"]
    ranges.forEach(range =>{
        console.log(range);
    });
});