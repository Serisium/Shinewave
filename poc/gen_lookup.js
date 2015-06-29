num_samples = 255
max_value = 255

function gamma(x) {
  return Math.pow(x, 1/0.45);
}

function sine(x) {
  return Math.sin(x*Math.PI/2)
}

for(var x = 0; x < num_samples; x++) {
  console.log(parseInt(gamma(sine(x/num_samples)) * max_value));
}
