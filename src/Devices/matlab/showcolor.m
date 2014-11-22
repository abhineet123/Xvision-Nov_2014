while 1
  [color]=grab_videre(1,1);
  imshow(permute(shiftdim(color,1),[2 1 3]));
  pause(0.01);
end
