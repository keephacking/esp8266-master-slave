import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import {MatToolbarModule} from '@angular/material/toolbar';
import {MatListModule} from '@angular/material/list';
const MODS=[MatToolbarModule,MatListModule];
@NgModule({
  imports: [
    CommonModule,
    ...MODS
  ],
  exports:[...MODS],
  declarations: []
})
export class SharedModule { }
