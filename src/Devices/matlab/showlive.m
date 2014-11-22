figure(1);
colormap(gray);
while 1
  [left,right]=grab_videre(1,0);
  subplot(1,2,1), image(double(left)');
  subplot(1,2,2), image(double(right)');
  pause(0.01);
end
