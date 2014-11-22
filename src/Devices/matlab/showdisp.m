figure(1);
colormap(gray);
while 1
  [disp]=grab_videre(1,2);
  corr_disp=double(disp);
  image(((corr_disp).*(corr_disp<65000))'/16)
  pause(0.01);
end
