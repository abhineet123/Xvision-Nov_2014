while 1
  [color]=grab_digit(1);
  image(permute(shiftdim(color,1),[2 1 3]));
  pause(0.01);
end
