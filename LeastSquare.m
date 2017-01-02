[consts_len, N, M] = textread('dim.txt', '%d %d %d\n');
imgSize = N*M;



for i=0:2
    [row_inds,col_inds,vals] = textread(sprintf('A%dM.txt', i), '%d %d %f\n');
    row_inds = row_inds +1 ;
    col_inds = col_inds +1 ;
    [b] = textread(sprintf('b%dM.txt', i), '%f\n');
    A=sparse(row_inds,col_inds,vals,consts_len,imgSize);
    new_vals=A\b; 
    Emaxima = reshape(new_vals, N, M);

    [row_inds,col_inds,vals] = textread(sprintf('A%dm.txt', i), '%d %d %f\n');
    row_inds = row_inds +1 ;
    col_inds = col_inds +1 ;
    [b] = textread(sprintf('b%dm.txt', i), '%f\n');
    A=sparse(row_inds,col_inds,vals,consts_len,imgSize);
    new_vals=A\b; 
    Eminima = reshape(new_vals, N, M);

    if i==0
        EMean(:,:,3) = (Eminima(:,:,1) + Emaxima(:,:,1))/2;
    end
    if i==1
        EMean(:,:,2) = (Eminima(:,:,1) + Emaxima(:,:,1))/2;
    end
    if i==2
        EMean(:,:,1) = (Eminima(:,:,1) + Emaxima(:,:,1))/2;
    end
    
    delete(sprintf('A%dM.txt', i));
    delete(sprintf('b%dM.txt', i));
    delete(sprintf('A%dm.txt', i));
    delete(sprintf('b%dm.txt', i));
end

delete('dim.txt');

figure
imshow(EMean);
